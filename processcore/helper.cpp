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

KAuth::ActionReply KSysGuardProcessListHelper::sendsignal(QVariantMap parameters) {
    KSysGuard::ProcessesLocal processes;
    int signal = qvariant_cast<int>(parameters.value("signal"));
    bool success = true;
    int numProcesses = parameters.value("pidcount").toInt();
    for (int i = 0; i < numProcesses; ++i) {
        success = processes.sendSignal(parameters.value(QString("pid%1").arg(i)).toULongLong(), signal) && success;
    }
    if(success)
        return KAuth::ActionReply::SuccessReply;
    else
        return KAuth::ActionReply::HelperErrorReply;
}

KAuth::ActionReply KSysGuardProcessListHelper::renice(QVariantMap parameters) {
    KSysGuard::ProcessesLocal processes;
    int niceValue = qvariant_cast<int>(parameters.value("nicevalue"));
    bool success = true;
    int numProcesses = parameters.value("pidcount").toInt();
    for (int i = 0; i < numProcesses; ++i) {
        success = processes.setNiceness(parameters.value(QString("pid%1").arg(i)).toULongLong(), niceValue) && success;
    }
    if(success)
        return KAuth::ActionReply::SuccessReply;
    else
        return KAuth::ActionReply::HelperErrorReply;
}

KAuth::ActionReply KSysGuardProcessListHelper::changeioscheduler(QVariantMap parameters) {
    KSysGuard::ProcessesLocal processes;
    int ioScheduler = qvariant_cast<int>(parameters.value("ioScheduler"));
    int ioSchedulerPriority = qvariant_cast<int>(parameters.value("ioSchedulerPriority"));
    bool success = true;
    int numProcesses = parameters.value("pidcount").toInt();
    for (int i = 0; i < numProcesses; ++i) {
        success = processes.setIoNiceness(parameters.value(QString("pid%1").arg(i)).toULongLong(), ioScheduler, ioSchedulerPriority) && success;
    }
    if(success)
        return KAuth::ActionReply::SuccessReply;
    else
        return KAuth::ActionReply::HelperErrorReply;

}
KAuth::ActionReply KSysGuardProcessListHelper::changecpuscheduler(QVariantMap parameters) {
    KSysGuard::ProcessesLocal processes;
    int cpuScheduler = qvariant_cast<int>(parameters.value("cpuScheduler"));
    int cpuSchedulerPriority = qvariant_cast<int>(parameters.value("cpuSchedulerPriority"));
    bool success = true;

    int numProcesses = parameters.value("pidcount").toInt();
    for (int i = 0; i < numProcesses; ++i) {
        success = processes.setScheduler(parameters.value(QString("pid%1").arg(i)).toULongLong(), cpuScheduler, cpuSchedulerPriority) && success;
    }
    if(success)
        return KAuth::ActionReply::SuccessReply;
    else
        return KAuth::ActionReply::HelperErrorReply;

}
KDE4_AUTH_HELPER_MAIN("org.kde.ksysguard.processlisthelper", KSysGuardProcessListHelper)

