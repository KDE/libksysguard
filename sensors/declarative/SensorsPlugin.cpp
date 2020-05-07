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

#include "SensorsPlugin.h"

#include "Sensor.h"
#include "SensorDataModel.h"
#include "SensorTreeModel.h"

#include <QQmlEngine>

using namespace KSysGuard;

void SensorsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.ksysguard.sensors"));

    qmlRegisterType<SensorDataModel>(uri, 1, 0, "SensorDataModel");
    qmlRegisterType<SensorTreeModel>(uri, 1, 0, "SensorTreeModel");
    qmlRegisterType<Sensor>(uri, 1, 0, "Sensor");
}
