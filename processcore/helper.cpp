/*
    KSysGuard, the KDE System Guard

	Copyright (c) 2009 John Tapsell <john.tapsell@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "helper.h"
#include "processes_local_p.h"

using namespace KAuth;

KSysGuardProcessListHelper::KSysGuardProcessListHelper()
{
    qRegisterMetaType<QList<long long> >();
}

/* The functions here run as ROOT.  So be careful.  DO NOT TRUST THE INPUTS TO BE SANE. */
#define GET_PID(i) parameters.value(QString("pid%1").arg(i), -1).toULongLong(); if(pid < 0) return ActionReply(ActionReply::HelperErrorType);
ActionReply KSysGuardProcessListHelper::sendsignal(QVariantMap parameters) {
    ActionReply reply(ActionReply::HelperErrorType);
    if(!parameters.contains("signal")) {
        reply.setErrorDescription("Internal error - no signal parameter was passed to the helper");
        reply.setErrorCode(static_cast<ActionReply::Error>(KSysGuard::Processes::InvalidPid));
        return reply;
    }
    if(!parameters.contains("pidcount")) {
        reply.setErrorDescription("Internal error - no pidcount parameter was passed to the helper");
        reply.setErrorCode(static_cast<ActionReply::Error>(KSysGuard::Processes::InvalidParameter));
        return reply;
    }

    KSysGuard::ProcessesLocal processes;
    int signal = qvariant_cast<int>(parameters.value("signal"));
    bool success = true;
    int numProcesses = parameters.value("pidcount").toInt();
    QStringList errorList;
    for (int i = 0; i < numProcesses; ++i) {
        qlonglong pid = GET_PID(i);
        bool successForThisPid = processes.sendSignal(pid, signal);
        if (!successForThisPid)
            errorList << QString::number(pid);
        success = successForThisPid && success;
    }
    if(success) {
        return ActionReply::SuccessReply();
    } else {
        reply.setErrorDescription(QString("Could not send signal to: ") + errorList.join(", "));
        reply.setErrorCode(static_cast<ActionReply::Error>(KSysGuard::Processes::Unknown));
        return reply;
    }
}

ActionReply KSysGuardProcessListHelper::renice(QVariantMap parameters) {
    if(!parameters.contains("nicevalue") || !parameters.contains("pidcount"))
        return ActionReply(ActionReply::HelperErrorType);

    KSysGuard::ProcessesLocal processes;
    int niceValue = qvariant_cast<int>(parameters.value("nicevalue"));
    bool success = true;
    int numProcesses = parameters.value("pidcount").toInt();
    for (int i = 0; i < numProcesses; ++i) {
        qlonglong pid = GET_PID(i);
        success = processes.setNiceness(pid, niceValue) && success;
    }
    if(success)
        return ActionReply::SuccessReply();
    else
        return ActionReply(ActionReply::HelperErrorType);
}

ActionReply KSysGuardProcessListHelper::changeioscheduler(QVariantMap parameters) {
    if(!parameters.contains("ioScheduler") || !parameters.contains("ioSchedulerPriority") || !parameters.contains("pidcount"))
        return ActionReply(ActionReply::HelperErrorType);

    KSysGuard::ProcessesLocal processes;
    int ioScheduler = qvariant_cast<int>(parameters.value("ioScheduler"));
    int ioSchedulerPriority = qvariant_cast<int>(parameters.value("ioSchedulerPriority"));
    bool success = true;
    int numProcesses = parameters.value("pidcount").toInt();
    for (int i = 0; i < numProcesses; ++i) {
        qlonglong pid = GET_PID(i);
        success = processes.setIoNiceness(pid, ioScheduler, ioSchedulerPriority) && success;
    }
    if(success)
        return ActionReply::SuccessReply();
    else
        return ActionReply(ActionReply::HelperErrorType);

}
ActionReply KSysGuardProcessListHelper::changecpuscheduler(QVariantMap parameters) {
    if(!parameters.contains("cpuScheduler") || !parameters.contains("cpuSchedulerPriority") || !parameters.contains("pidcount"))
        return ActionReply(ActionReply::HelperErrorType);

    KSysGuard::ProcessesLocal processes;
    int cpuScheduler = qvariant_cast<int>(parameters.value("cpuScheduler"));
    int cpuSchedulerPriority = qvariant_cast<int>(parameters.value("cpuSchedulerPriority"));
    bool success = true;

    int numProcesses = parameters.value("pidcount").toInt();
    for (int i = 0; i < numProcesses; ++i) {
        qlonglong pid = GET_PID(i);
        success = processes.setScheduler(pid, cpuScheduler, cpuSchedulerPriority) && success;
    }
    if(success)
        return ActionReply::SuccessReply();
    else
        return ActionReply(ActionReply::HelperErrorType);

}
KAUTH_HELPER_MAIN("org.kde.ksysguard.processlisthelper", KSysGuardProcessListHelper)

