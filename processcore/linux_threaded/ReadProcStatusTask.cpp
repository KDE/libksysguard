/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ReadProcStatusTask.h"

#include <QFile>

using namespace KSysGuard;

ReadProcStatusTask::ReadProcStatusTask(const QString &dir)
    : Task(dir + QStringLiteral("/status"))
{
}

void KSysGuard::ReadProcStatusTask::updateProcess(KSysGuard::Process* process)
{
    if (process->command().isEmpty()) {
        process->setName(m_processName);
    }

    process->setNoNewPrivileges(m_noNewPrivileges);

    process->setUid(m_uid);
    process->setEuid(m_euid);
    process->setSuid(m_suid);
    process->setFsuid(m_fsuid);

    process->setGid(m_gid);
    process->setEgid(m_egid);
    process->setSgid(m_sgid);
    process->setFsgid(m_fsgid);

    process->setTracerpid(m_tracerPid);
    process->setNumThreads(m_threadCount);
}

bool ReadProcStatusTask::processLine(const QByteArray& data)
{
    if (data.startsWith("Name:")) {
        m_processName = QString::fromLocal8Bit(data.mid(sizeof("Name:")).trimmed());
        m_entriesFound++;

    } else if (data.startsWith("NoNewPrivs:")) {
        m_noNewPrivileges = data.mid(sizeof("NoNewPrivs:")).toInt();
        m_entriesFound++;

    } else if (data.startsWith("Uid:")) {
        auto parts = data.mid(sizeof("Uid:")).split('\t');
        if (parts.size() == 4) {
            m_uid = parts.at(0).toLongLong();
            m_euid = parts.at(1).toLongLong();
            m_suid = parts.at(2).toLongLong();
            m_fsuid = parts.at(3).toLongLong();
        }
        m_entriesFound++;

    } else if (data.startsWith("Gid:")) {
        auto parts = data.mid(sizeof("Gid:")).split('\t');
        if (parts.size() == 4) {
            m_gid = parts.at(0).toLongLong();
            m_egid = parts.at(1).toLongLong();
            m_sgid = parts.at(2).toLongLong();
            m_fsgid = parts.at(3).toLongLong();
        }
        m_entriesFound++;

    } else if (data.startsWith("TracerPid:")) {
        m_tracerPid = data.mid(sizeof("TracerPid:")).toLongLong();
        if (m_tracerPid == 0) {
            m_tracerPid = -1;
        }
        m_entriesFound++;

    } else if (data.startsWith("Threads:")) {
        m_threadCount = data.mid(sizeof("Threads:")).toInt();
        m_entriesFound++;
    }

    if (m_entriesFound >= 6) {
        return false;
    }

    return true;
}

