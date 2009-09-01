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
#include <QDebug>
#include "processes_local_p.h"


/* The functions here run as ROOT.  So be careful. */

KAuth::ActionReply KSysGuardProcessListHelper::sendsignal(QVariantMap parameters) {
    qDebug() << "HERERERER";
    KSysGuard::ProcessesLocal processes;
    QList< long long> pids = parameters.value("pids").value<QList<long long> >();
    int signal = qvariant_cast<int>(parameters.value("signal"));
    bool success = true;
    for (int i = 0; i < pids.size(); ++i) {
        success = processes.sendSignal(pids.at(i), signal);
    }
    if(success)
        return KAuth::ActionReply::SuccessReply;
    else
        return KAuth::ActionReply::HelperErrorReply;
}

KDE4_AUTH_HELPER_MAIN("org.kde.ksysguard.processlisthelper", KSysGuardProcessListHelper);

