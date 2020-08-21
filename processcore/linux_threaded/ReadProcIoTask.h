/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "Task.h"

namespace KSysGuard {

class Q_DECL_EXPORT ReadProcIoTask : public Task
{
public:
    ReadProcIoTask(const QString &dir);

    void updateProcess(Process *process) override;

private:
    bool processLine(const QByteArray &data) override;

    qlonglong m_charactersRead = -1;
    qlonglong m_charactersWritten = -1;
    qlonglong m_readSyscalls = -1;
    qlonglong m_writeSyscalls = -1;
    qlonglong m_bytesRead = -1;
    qlonglong m_bytesWritten = -1;
};

} // namespace KSysGuard
