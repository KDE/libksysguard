/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "Task.h"

#include "process.h"

namespace KSysGuard {

class Q_DECL_EXPORT GetNicenessTask : public Task
{
public:
    GetNicenessTask(pid_t pid);

    void updateProcess(Process *process) override;

    void run() override;

private:
    bool processLine(const QByteArray &data) override;

    pid_t m_pid = -1;
    Process::Scheduler m_scheduler = Process::Other;
    int m_priority = 0;

    Process::IoPriorityClass m_ioScheduler = Process::None;
    int m_ioPriority = -1;
};

} // namespace KSysGuard
