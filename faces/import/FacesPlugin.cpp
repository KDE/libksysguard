/*
    Copyright (C) 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
    Copyright (C) 2020 Marco Martin <mart@kde.org>

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

#include "FacesPlugin.h"

#include "SensorDataModel.h"
#include "SensorTreeModel.h"
#include "Sensor.h"
#include "SensorFace_p.h"
#include "SensorFaceController.h"

#include <KDeclarative/ConfigPropertyMap>

#include <QQmlEngine>

using namespace KSysGuard;

void FacesPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.ksysguard.faces"));

    qmlRegisterType<SensorFace>(uri, 1, 0, "AbstractSensorFace");
    qmlRegisterUncreatableType<SensorFaceController>(uri, 1, 0, "SensorFaceController", QStringLiteral("It's not possible to create objects of type SensorFaceController"));
    qmlRegisterAnonymousType<KDeclarative::ConfigPropertyMap>(uri, 1);
}
