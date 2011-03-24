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

KSysGuardProcessListHelper::KSysGuardProcessListHelper()
{
    qRegisterMetaType<QList<long long> >();
}

/* The functions here run as ROOT.  So be careful.  DO NOT TRUST THE INPUTS TO BE SANE. */
#define GET_PID(i) parameters.value(QString("pid%1").arg(i), -1).toULongLong(); if(pid < 0) return KAuth::ActionReply::HelperErrorReply;
KAuth::ActionReply KSysGuardProcessListHelper::sendsignal(QVariantMap parameters) {
    KAuth::ActionReply errorReply(KAuth::ActionReply::HelperError);
    if(!parameters.contains("signal")) {
        errorReply.setErrorDescription("Internal error - no signal parameter was passed to the helper");
        errorReply.setErrorCode(1);
        return errorReply;
    }
    if(!parameters.contains("pidcount")) {
        errorReply.setErrorDescription("Internal error - no pidcount parameter was passed to the helper");
        errorReply.setErrorCode(2);
        return errorReply;
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
        return KAuth::ActionReply::SuccessReply;
    } else {
        errorReply.setErrorDescription(QString("Could not send signal to: ") + errorList.join(", "));
        errorReply.setErrorCode(0);
        return errorReply;
    }
}

KAuth::ActionReply KSysGuardProcessListHelper::renice(QVariantMap parameters) {
    if(!parameters.contains("nicevalue") || !parameters.contains("pidcount"))
        return KAuth::ActionReply::HelperErrorReply;

    KSysGuard::ProcessesLocal processes;
    int niceValue = qvariant_cast<int>(parameters.value("nicevalue"));
    bool success = true;
    int numProcesses = parameters.value("pidcount").toInt();
    for (int i = 0; i < numProcesses; ++i) {
        qlonglong pid = GET_PID(i);
        success = processes.setNiceness(pid, niceValue) && success;
    }
    if(success)
        return KAuth::ActionReply::SuccessReply;
    else
        return KAuth::ActionReply::HelperErrorReply;
}

KAuth::ActionReply KSysGuardProcessListHelper::changeioscheduler(QVariantMap parameters) {
    if(!parameters.contains("ioScheduler") || !parameters.contains("ioSchedulerPriority") || !parameters.contains("pidcount"))
        return KAuth::ActionReply::HelperErrorReply;

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
        return KAuth::ActionReply::SuccessReply;
    else
        return KAuth::ActionReply::HelperErrorReply;

}
KAuth::ActionReply KSysGuardProcessListHelper::changecpuscheduler(QVariantMap parameters) {
    if(!parameters.contains("cpuScheduler") || !parameters.contains("cpuSchedulerPriority") || !parameters.contains("pidcount"))
        return KAuth::ActionReply::HelperErrorReply;

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
        return KAuth::ActionReply::SuccessReply;
    else
        return KAuth::ActionReply::HelperErrorReply;

}
KDE4_AUTH_HELPER_MAIN("org.kde.ksysguard.processlisthelper", KSysGuardProcessListHelper)

