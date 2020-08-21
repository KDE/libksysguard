/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "Task.h"

namespace KSysGuard {

class ReadProcCGroupTask : public Task
{
public:
    ReadProcCGroupTask(const QString &dir);

    void updateProcess(Process *process) override;

private:
    bool processLine(const QByteArray &data) override;

    QString m_cgroup;
};

} // namespace KSysGuard
