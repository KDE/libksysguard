/*
 *   SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QMetaType>
#include <qqmlintegration.h>

#include "process.h"

// A helper that exposes Process specific enums to QML.
namespace KSysGuardProcess
{

Q_NAMESPACE
QML_NAMED_ELEMENT(Process)

// Unfortunately, `using enum` doesn't work with declarative registration so
// we need to manually copy the enum elements here.
enum ProcessStatus {
    Running = KSysGuard::Process::Running,
    Sleeping = KSysGuard::Process::Sleeping,
    DiskSleep = KSysGuard::Process::DiskSleep,
    Zombie = KSysGuard::Process::Zombie,
    Stopped = KSysGuard::Process::Stopped,
    Paging = KSysGuard::Process::Paging,
    Ended = KSysGuard::Process::Ended,
    OtherStatus = KSysGuard::Process::OtherStatus,
};
Q_ENUM_NS(ProcessStatus)

enum IoPriorityClass {
    None = KSysGuard::Process::None,
    RealTime = KSysGuard::Process::RealTime,
    BestEffort = KSysGuard::Process::BestEffort,
    Idle = KSysGuard::Process::Idle,
};
Q_ENUM_NS(IoPriorityClass)

enum Scheduler {
    Other = KSysGuard::Process::Other,
    Fifo = KSysGuard::Process::Fifo,
    RoundRobin = KSysGuard::Process::RoundRobin,
    Batch = KSysGuard::Process::Batch,
    SchedulerIdle = KSysGuard::Process::SchedulerIdle,
    Interactive = KSysGuard::Process::Interactive,
};
Q_ENUM_NS(Scheduler)

}
