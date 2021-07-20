/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SensorsPlugin.h"

#include "Sensor.h"
#include "SensorDataModel.h"
#include "SensorTreeModel.h"
#include "SensorUnitModel.h"

#include <QQmlEngine>

using namespace KSysGuard;

void SensorsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.ksysguard.sensors"));

    qmlRegisterType<SensorDataModel>(uri, 1, 0, "SensorDataModel");
    qmlRegisterType<SensorTreeModel>(uri, 1, 0, "SensorTreeModel");
    qmlRegisterType<Sensor>(uri, 1, 0, "Sensor");
    qmlRegisterType<SensorUnitModel>(uri, 1, 0, "SensorUnitModel");
}
