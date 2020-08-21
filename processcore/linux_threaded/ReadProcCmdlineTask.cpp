/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ReadProcCmdlineTask.h"

using namespace KSysGuard;

ReadProcCmdlineTask::ReadProcCmdlineTask(const QString &dir)
    : Task(dir + QStringLiteral("/cmdline"))
{
}

void KSysGuard::ReadProcCmdlineTask::updateProcess(KSysGuard::Process* process)
{
    process->setCommand(m_commandLine);
}

bool ReadProcCmdlineTask::processLine(const QByteArray& data)
{
    m_commandLine = QString::fromLocal8Bit(data);

    if (!m_commandLine.isEmpty()) {
        auto processEnd = m_commandLine.indexOf(QLatin1Char('\0'));
        auto processStart = m_commandLine.lastIndexOf(QLatin1Char('/'), processEnd);
        m_processName = m_commandLine.mid(processStart, processEnd - processStart);
        m_commandLine.replace(QLatin1Char('\0'), QLatin1Char(' '));
    }

    return true;
}
