/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ReadProcStatTask.h"

#include <thread>
#include <unistd.h>

using namespace KSysGuard;

ReadProcStatTask::ReadProcStatTask(const QString &dir)
    : Task(dir + QStringLiteral("/stat"))
{
}

void ReadProcStatTask::updateProcess(KSysGuard::Process* process)
{
    process->setStatus(m_state);
    process->setTty(m_tty);
    process->setUserTime(m_userTime);
    process->setSysTime(m_systemTime);
    process->setStartTime(m_startTime);
}

bool ReadProcStatTask::processLine(const QByteArray& data)
{
    auto parts = data.split(' ');
    if (parts.length() < 23) {
        return false;
    }

    setState(parts.at(2));
    setTty(parts.at(6));

    m_userTime = parts.at(13).toLongLong();
    m_systemTime = parts.at(14).toLongLong();
    m_startTime = parts.at(21).toLongLong();

    m_vmSize = parts.at(22).toLongLong() / 1024;

    // Value in file is in number of pages, convert it to KiB.
    m_vmRSS = parts.at(23).toLongLong() * (sysconf(_SC_PAGESIZE) / 1024);

    return false;
}

void ReadProcStatTask::setState(const QByteArray& data)
{
    switch(data.at(0)) {
        case 'R':
            m_state = Process::Running;
            break;
        case 'S':
            m_state = Process::Sleeping;
            break;
        case 'D':
            m_state = Process::DiskSleep;
            break;
        case 'Z':
            m_state = Process::Zombie;
            break;
        case 'T':
            m_state = Process::Stopped;
            break;
        case 'W':
            m_state = Process::Paging;
            break;
        default:
            m_state = Process::OtherStatus;
            break;
    }
}

void ReadProcStatTask::setTty(const QByteArray& data)
{
    int ttyNo = data.toInt();

    int major = ttyNo >> 8;
    int minor = ttyNo & 0xff;

    switch(major) {
        case 136:
            m_tty = QByteArray("pts/") + QByteArray::number(minor);
            break;
        case 5:
            m_tty = QByteArray("tty");
            break;
        case 4:
            if(minor < 64) {
                m_tty = QByteArray("tty") + QByteArray::number(minor);
            } else {
                m_tty = QByteArray("ttyS") + QByteArray::number(minor - 64);
            }
            break;
        default:
            m_tty = QByteArray{};
    }
}
