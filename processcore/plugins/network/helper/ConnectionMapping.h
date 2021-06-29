/*
 * This file is part of KSysGuard.
 * Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#ifndef CONNECTIONMAPPING_H
#define CONNECTIONMAPPING_H

#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <thread>
#include <mutex>
#include <atomic>

#include <netlink/socket.h>

#include "Packet.h"

struct nl_msg;

/**
 * @todo write docs
 */
class ConnectionMapping
{
public:
    using inode_t = uint32_t;
    using pid_t = int;

    struct PacketResult {
        pid_t pid = 0;
        Packet::Direction direction;
    };

    ConnectionMapping();

    PacketResult pidForPacket(const Packet &packet);

    void stop();

private:
    struct State
    {
        State &operator= (const State &other)
        {
            addressToInode = other.addressToInode;
            inodeToPid = other.inodeToPid;
            addresses = other.addresses;
            inodes = other.inodes;

            return *this;
        }

        inline void reset()
        {
            addresses.clear();
            inodes.clear();
            changed = false;
        }

        std::unordered_map<Packet::Address, inode_t> addressToInode;
        std::unordered_map<inode_t, pid_t> inodeToPid;

        std::unordered_set<Packet::Address> addresses;
        std::unordered_set<inode_t> inodes;

        bool changed = false;
    };

    void loop();
    bool dumpSockets(nl_sock *socket);
    bool dumpSockets(nl_sock *socket, int inet_family, int protocol);
    void parsePid();

    State m_oldState;
    State m_newState;

    std::thread m_thread;
    std::atomic_bool m_running;
    std::mutex m_mutex;

    friend int parseInetDiagMesg(struct nl_msg *msg, void *arg);
};

#endif // CONNECTIONMAPPING_H
