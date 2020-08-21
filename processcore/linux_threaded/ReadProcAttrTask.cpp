/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ReadProcAttrTask.h"

using namespace KSysGuard;

ReadProcAttrTask::ReadProcAttrTask(const QString &dir)
    : Task(dir + QStringLiteral("/attr/current"))
{
}

void KSysGuard::ReadProcAttrTask::updateProcess(KSysGuard::Process* process)
{
    process->setMACContext(m_securityContext);
}

bool ReadProcAttrTask::processLine(const QByteArray& data)
{
    m_securityContext = QString::fromLocal8Bit(data.trimmed());
    return false;
}
