/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ReadProcCGroupTask.h"

using namespace KSysGuard;

ReadProcCGroupTask::ReadProcCGroupTask(const QString &dir)
    : Task(dir + QStringLiteral("/cgroup"))
{
}

void KSysGuard::ReadProcCGroupTask::updateProcess(KSysGuard::Process* process)
{
    process->setCGroup(m_cgroup);
}

bool ReadProcCGroupTask::processLine(const QByteArray& data)
{
    if (data.startsWith("0::")) {
        m_cgroup = QString::fromLocal8Bit(data.mid(sizeof("0::")).trimmed());
        return false;
    }

    return true;
}
