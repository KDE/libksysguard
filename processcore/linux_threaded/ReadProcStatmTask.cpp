/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ReadProcStatmTask.h"

#include <unistd.h>

using namespace KSysGuard;

ReadProcStatmTask::ReadProcStatmTask(const QString &dir)
    : Task(dir + QStringLiteral("/cgroup"))
{
}

void KSysGuard::ReadProcStatmTask::updateProcess(KSysGuard::Process* process)
{
    process->setVmURSS(m_urss);
}

bool ReadProcStatmTask::processLine(const QByteArray& data)
{
    auto parts = data.split(' ');
    if (parts.size() < 7) {
        return true;
    }

    auto rss = parts.at(1).toLongLong();
    auto shared = parts.at(2).toLongLong();

    m_urss = (rss - shared) * sysconf(_SC_PAGESIZE) / 1024;

    return true;
}
