/*
    This file is part of KSysGuard.
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CONNECTIONMAPPING_H
#define CONNECTIONMAPPING_H

#include <unordered_map>
#include <unordered_set>
#include <regex>

#include <netlink/socket.h>

#include "Packet.h"

struct nl_msg;

/**
 * @todo write docs
 */
class ConnectionMapping
{
public:
    struct PacketResult {
        int pid = 0;
        Packet::Direction direction;
    };

    ConnectionMapping();

    PacketResult pidForPacket(const Packet &packet);

private:
    void getSocketInfo();
    bool dumpSockets();
    bool dumpSockets(int inet_family, int protocol);
    void parseSockets();
    void parsePid();
    void parseSocketFile(const char* fileName);

    std::unordered_map<Packet::Address, int> m_localToINode;
    std::unordered_map<int, int> m_inodeToPid;
    std::unordered_set<int> m_inodes;
    std::unordered_set<int> m_pids;
    std::unique_ptr<nl_sock, decltype(&nl_socket_free)> m_socket;
    std::regex m_socketFileMatch;

    friend int parseInetDiagMesg(struct nl_msg *msg, void *arg);
};

#endif // CONNECTIONMAPPING_H
