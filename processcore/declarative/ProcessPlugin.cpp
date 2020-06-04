/*
    Copyright (C) 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

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

#include "ProcessPlugin.h"

#include <QQmlEngine>

#include "application_data_model.h"
#include "process_controller.h"
#include "process_data_model.h"
#include "process_attribute_model.h"

#include "ProcessEnums.h"


using namespace KSysGuard;

void ProcessPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.ksysguard.process"));

    qRegisterMetaType<KSysGuard::ProcessController::Signal>();
    qRegisterMetaType<KSysGuard::ProcessController::Result>();
    qRegisterMetaType<KSysGuardProcess::ProcessStatus>();
    qRegisterMetaType<KSysGuardProcess::IoPriorityClass>();
    qRegisterMetaType<KSysGuardProcess::Scheduler>();

    qmlRegisterType<ProcessController>(uri, 1, 0, "ProcessController");
    qmlRegisterUncreatableMetaObject(KSysGuardProcess::staticMetaObject, uri, 1, 0, "Process", QStringLiteral("Contains process enums"));
    qmlRegisterType<ProcessDataModel>(uri, 1, 0, "ProcessDataModel");
    qmlRegisterUncreatableType<ProcessAttributeModel>(uri, 1, 0, "ProcessAttributeModel", QStringLiteral("Available through ProcessDataModel"));
    qmlRegisterType<ApplicationDataModel>(uri, 1, 0, "ApplicationDataModel");

}
