/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "Task.h"

namespace KSysGuard {

class Q_DECL_EXPORT ReadProcCmdlineTask : public Task
{
public:
    ReadProcCmdlineTask(const QString &dir);

    void updateProcess(Process *process) override;

private:
    bool processLine(const QByteArray &data) override;

    QString m_commandLine;
    QString m_processName;
};

} // namespace KSysGuard
