/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "Task.h"

namespace KSysGuard {

class ReadProcStatusTask : public Task
{
public:
    ReadProcStatusTask(const QString &dir);

    void updateProcess(Process *process) override;

private:
    bool processLine(const QByteArray &data) override;

    QString m_processName;

    qint64 m_uid = 0;
    qint64 m_euid = 0;
    qint64 m_suid = 0;
    qint64 m_fsuid = 0;

    qint64 m_gid = 0;
    qint64 m_egid = 0;
    qint64 m_sgid = 0;
    qint64 m_fsgid = 0;

    qint64 m_tracerPid = -1;
    int m_threadCount = 0;
    int m_noNewPrivileges = 0;

    int m_entriesFound = 0;
};

} // namespace KSysGuard
