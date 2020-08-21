/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "Task.h"

#include "process.h"

namespace KSysGuard {

class Q_DECL_EXPORT ReadProcStatTask : public Task
{
public:
    ReadProcStatTask(const QString &dir);

    void updateProcess(Process *process) override;

private:
    bool processLine(const QByteArray &data) override;

    void setState(const QByteArray &data);
    void setTty(const QByteArray &data);

    Process::ProcessStatus m_state = Process::OtherStatus;
    QByteArray m_tty;

    qlonglong m_userTime = -1;
    qlonglong m_systemTime = -1;
    qlonglong m_startTime = -1;

    qlonglong m_vmSize = -1;
    qlonglong m_vmRSS = -1;
};

} // namespace KSysGuard
