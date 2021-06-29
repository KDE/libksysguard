/*
 * This file is part of KSysGuard.
 * Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 * Copyright 2020 David Redondo <kde@david-redondo.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ConnectionMapping.h"

#include <fstream>
#include <iostream>
#include <charconv>

#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/inet_diag.h>
#include <linux/sock_diag.h>

#include <netlink/msg.h>
#include <netlink/netlink.h>

using namespace std::string_literals;

template <typename Key, typename Value>
inline void cleanupOldEntries(const std::unordered_set<Key> &keys, std::unordered_map<Key, Value> &map)
{
    for (auto itr = map.begin(); itr != map.end();) {
        if (keys.find(itr->first) == keys.end()) {
            itr = map.erase(itr);
        } else {
            itr++;
        }
    }
}

ConnectionMapping::inode_t toInode(const std::string_view &view)
{
    ConnectionMapping::inode_t value;
    if (auto status = std::from_chars(view.data(), view.data() + view.length(), value); status.ec == std::errc()) {
        return value;
    }
    return std::numeric_limits<ConnectionMapping::inode_t>::max();
}

int parseInetDiagMesg(struct nl_msg *msg, void *arg) 
{
    auto self = static_cast<ConnectionMapping*>(arg);
    struct nlmsghdr *nlh = nlmsg_hdr(msg);
    auto inetDiagMsg = static_cast<inet_diag_msg*>(nlmsg_data(nlh));
    Packet::Address localAddress;
    if (inetDiagMsg->idiag_family == AF_INET) {
        // I expected to need ntohl calls here and bewlow for src but comparing to values gathered
        // by parsing proc they are not needed and even produce wrong results
        localAddress.address[3] = inetDiagMsg->id.idiag_src[0];
    } else if (inetDiagMsg->id.idiag_src[0] == 0 && inetDiagMsg->id.idiag_src[1] == 0 &&  inetDiagMsg->id.idiag_src[2] == 0xffff0000) {
         // Some applications (like Steam) use ipv6 sockets with ipv4.
        // This results in ipv4 addresses that end up in the tcp6 file.
        // They seem to start with 0000000000000000FFFF0000, so if we
        // detect that, assume it is ipv4-over-ipv6.
        localAddress.address[3] = inetDiagMsg->id.idiag_src[3];

    } else {
        std::memcpy(localAddress.address.data(), inetDiagMsg->id.idiag_src, sizeof(localAddress.address));
    }
    localAddress.port = ntohs(inetDiagMsg->id.idiag_sport);

    if (self->m_newState.addressToInode.find(localAddress) == self->m_newState.addressToInode.end()) {
        self->m_newState.addressToInode.emplace(localAddress, inetDiagMsg->idiag_inode);
        self->m_newState.changed = true;
    }

    self->m_newState.addresses.insert(localAddress);
    self->m_newState.inodes.insert(inetDiagMsg->idiag_inode);

    return NL_OK;
}

ConnectionMapping::ConnectionMapping()
{
    m_thread = std::thread(&ConnectionMapping::loop, this);
}

ConnectionMapping::PacketResult ConnectionMapping::pidForPacket(const Packet &packet)
{
    std::lock_guard<std::mutex> lock{m_mutex};

    PacketResult result;

    auto sourceInode = m_oldState.addressToInode.find(packet.sourceAddress());
    auto destInode = m_oldState.addressToInode.find(packet.destinationAddress());

    if (sourceInode == m_oldState.addressToInode.end() && destInode == m_oldState.addressToInode.end()) {
        return result;
    }

    auto inode = m_oldState.addressToInode.end();
    if (sourceInode != m_oldState.addressToInode.end()) {
        result.direction = Packet::Direction::Outbound;
        inode = sourceInode;
    } else {
        result.direction = Packet::Direction::Inbound;
        inode = destInode;
    }

    auto pid = m_oldState.inodeToPid.find((*inode).second);
    if (pid == m_oldState.inodeToPid.end()) {
        result.pid = -1;
    } else {
        result.pid = (*pid).second;
    }
    return result;
}

void ConnectionMapping::stop()
{
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void ConnectionMapping::loop()
{
    std::unique_ptr<nl_sock, decltype(&nl_socket_free)> socket{nl_socket_alloc(), nl_socket_free};

    nl_connect(socket.get(), NETLINK_SOCK_DIAG);
    nl_socket_modify_cb(socket.get(), NL_CB_VALID, NL_CB_CUSTOM, &parseInetDiagMesg, this);

    m_running = true;

    while (m_running) {
        m_newState.reset();

        dumpSockets(socket.get());

        if (m_newState.changed) {
            parsePid();
        }

        cleanupOldEntries(m_newState.addresses, m_newState.addressToInode);
        cleanupOldEntries(m_newState.inodes, m_newState.inodeToPid);

        {
            std::lock_guard<std::mutex> lock{m_mutex};
            m_oldState = m_newState;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

bool ConnectionMapping::dumpSockets(nl_sock *socket)
{
    for (auto family : {AF_INET, AF_INET6}) {
        for (auto protocol : {IPPROTO_TCP, IPPROTO_UDP}) {
            if (!dumpSockets(socket, family, protocol)) {
                return false;
            }
        }
    }
    return true;
}

bool ConnectionMapping::dumpSockets(nl_sock *socket, int inet_family, int protocol)
{
    inet_diag_req_v2 inet_request;
    inet_request.id = {};
    inet_request.sdiag_family = inet_family;
    inet_request.sdiag_protocol = protocol;
    inet_request.idiag_states = -1;
    if (nl_send_simple(socket, SOCK_DIAG_BY_FAMILY, NLM_F_DUMP | NLM_F_REQUEST, &inet_request, sizeof(inet_diag_req_v2)) < 0) {
        return false;
    }
    if (nl_recvmsgs_default(socket) != 0) {
        return false;
    }
    return true;
}

void ConnectionMapping::parsePid()
{
    std::vector<pid_t> pids;
    // Reserve an initial amount since we know there will be at least a bunch
    // of pids.
    pids.reserve(500);

    auto dir = opendir("/proc");

    // The only way to get a list of PIDs is to list the contents of /proc.
    // Any directory with a numeric name corresponds to a process and its PID.
    dirent *entry = nullptr;
    while ((entry = readdir(dir))) {
        if (entry->d_type != DT_DIR) {
            continue;
        }

        if (entry->d_name[0] >= '0' && entry->d_name[0] <= '9') {
            pids.push_back(std::stoi(entry->d_name));
        }
    }
    closedir(dir);

    std::array<char, 100> buffer;

    for (auto pid : pids) {
        // We need to list the contents of a subdirectory of the PID directory.
        // We create the path by replacing the _ part with the actual PID that
        // we want to list. This uses 16 characters so we know the destination
        // string will be large enough.
        auto fdPath = "/proc/________________/fd"s;
        fdPath.replace(6, 16, std::to_string(pid));

        auto dir = opendir(fdPath.data());
        if (dir == NULL) {
            continue;
        }

        std::vector<std::string> links;
        dirent *fd = nullptr;
        while ((fd = readdir(dir))) {
            if (fd->d_type != DT_LNK) {
                continue;
            }

            // Clear the link destination buffer by filling it with \0 bytes.
            buffer.fill('\0');

            // /proc/PID/fd contains symlinks for each open fd in the process.
            // The symlink target contains information about what the fd is about.
            readlinkat(dirfd(dir), fd->d_name, buffer.data(), 99);

            auto view = std::string_view(buffer.data(), 100);

            // In this case, we are only interested in sockets, for which the
            // symlink target starts with 'socket:', followed by the inode
            // number in square brackets.
            if (view.compare(0, 7, "socket:") != 0) {
                continue;
            }

            // Strip off the leading "socket:" part and the opening bracket,
            // then convert that to an inode number.
            auto inode = toInode(view.substr(8));
            if (inode != std::numeric_limits<inode_t>::max()) {
                m_newState.inodeToPid[inode] = pid;
            }
        }

        closedir(dir);
    }
}
