/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "Task.h"

namespace KSysGuard {

class Q_DECL_EXPORT ReadProcSmapsTask : public Task
{
public:
    ReadProcSmapsTask(const QString &dir);

    void updateProcess(Process *process) override;

private:
    bool processLine(const QByteArray &data) override;

    qlonglong m_pss = 0;
};

} // namespace KSysGuard
