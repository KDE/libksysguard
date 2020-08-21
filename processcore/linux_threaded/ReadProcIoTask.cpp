/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ReadProcIoTask.h"

using namespace KSysGuard;

ReadProcIoTask::ReadProcIoTask(const QString &dir)
    : Task(dir + QStringLiteral("/io"))
{
}

void ReadProcIoTask::updateProcess(KSysGuard::Process *process)
{
    process->setIoCharactersRead(m_charactersRead);
    process->setIoCharactersWritten(m_charactersWritten);
    process->setIoReadSyscalls(m_readSyscalls);
    process->setIoWriteSyscalls(m_writeSyscalls);
    process->setIoCharactersActuallyRead(m_bytesRead);
    process->setIoCharactersActuallyWritten(m_bytesWritten);
}

bool ReadProcIoTask::processLine(const QByteArray &data)
{
    if (data.startsWith("rchar:")) {
        m_charactersRead = data.mid(sizeof("rchar:")).toLongLong();
    } else if (data.startsWith("wchar:")) {
        m_charactersWritten = data.mid(sizeof("wchar:")).toLongLong();
    } else if (data.startsWith("syscr:")) {
        m_readSyscalls = data.mid(sizeof("syscr:")).toLongLong();
    } else if (data.startsWith("syscw:")) {
        m_writeSyscalls = data.mid(sizeof("syscw:")).toLongLong();
    } else if (data.startsWith("read_bytes:")) {
        m_bytesRead = data.mid(sizeof("read_bytes:")).toLongLong();
    } else if (data.startsWith("write_bytes:")) {
        m_bytesWritten = data.mid(sizeof("write_bytes:")).toLongLong();
    }

    return true;
}
