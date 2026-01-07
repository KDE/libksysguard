/*
 *  SPDX-FileCopyrightText: 2025 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <qqmlintegration.h>

#include <signal.h>

namespace KSysGuard
{
namespace Enums
{

namespace Result
{
Q_NAMESPACE
QML_ELEMENT

/**
 * What kind of result a call to one of the ProcessController methods had.
 */
enum Result {
    Unknown, ///< Something happened, we just do not know what.
    Success, ///< Everything went alright.
    InsufficientPermissions, ///< Some processes require privileges to modify and we failed getting those.
    NoSuchProcess, ///< Tried to modify a process that no longer exists.
    Unsupported, ///< The specified action is not supported.
    UserCancelled, ///< The user cancelled the action, usually when requesting privileges.
    Error, ///< An error occurred when requesting privileges.
};
Q_ENUM_NS(Result)
}

namespace Signal
{
Q_NAMESPACE
QML_ELEMENT

enum Signal {
    StopSignal = SIGSTOP,
    ContinueSignal = SIGCONT,
    HangupSignal = SIGHUP,
    InterruptSignal = SIGINT,
    TerminateSignal = SIGTERM,
    KillSignal = SIGKILL,
    User1Signal = SIGUSR1,
    User2Signal = SIGUSR2
};
Q_ENUM_NS(Signal)
}

namespace ProcessStatus
{
Q_NAMESPACE
QML_ELEMENT

enum ProcessStatus {
    Running,
    Sleeping,
    DiskSleep,
    Zombie,
    Stopped,
    Paging,
    Ended,
    OtherStatus = 99
};
Q_ENUM_NS(ProcessStatus)
}

namespace IoPriority
{
Q_NAMESPACE
QML_ELEMENT

enum IoPriority {
    None,
    RealTime,
    BestEffort,
    Idle
};
Q_ENUM_NS(IoPriority)
}

namespace Scheduler
{
Q_NAMESPACE
QML_ELEMENT

enum Scheduler {
    Other,
    Fifo,
    RoundRobin,
    Batch,
    SchedulerIdle,
    Interactive
};
Q_ENUM_NS(Scheduler)
}

}

}
