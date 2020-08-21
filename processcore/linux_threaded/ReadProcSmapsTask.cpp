/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ReadProcSmapsTask.h"

using namespace KSysGuard;

ReadProcSmapsTask::ReadProcSmapsTask(const QString &dir)
    : Task(dir + QStringLiteral("/smaps_rollup"))
{
}

void KSysGuard::ReadProcSmapsTask::updateProcess(KSysGuard::Process* process)
{
    process->setVmPSS(m_pss);
}

bool ReadProcSmapsTask::processLine(const QByteArray& data)
{
//     if (data.startsWith("Pss:")) {
//         m_pss += data.mid(sizeof("Pss:")).toLongLong();
//         return false;
//     }
    return true;
}
