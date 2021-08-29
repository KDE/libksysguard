/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "FacesPlugin.h"

#include "FaceLoader.h"
#include "Sensor.h"
#include "SensorDataModel.h"
#include "SensorFaceController.h"
#include "SensorFace_p.h"
#include "SensorTreeModel.h"

#include <KDeclarative/ConfigPropertyMap>

#include <QQmlEngine>
#include <QTransposeProxyModel>

using namespace KSysGuard;

void FacesPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.ksysguard.faces"));

    qmlRegisterType<KSysGuard::SensorFace>(uri, 1, 0, "AbstractSensorFace");
    qmlRegisterUncreatableType<KSysGuard::SensorFaceController>(uri,
                                                                1,
                                                                0,
                                                                "SensorFaceController",
                                                                QStringLiteral("It's not possible to create objects of type SensorFaceController"));
    qmlRegisterAnonymousType<KDeclarative::ConfigPropertyMap>(uri, 1);
    qmlRegisterType<QTransposeProxyModel>("org.kde.ksysguard.faces.private", 1, 0, "QTransposeProxyModel");

    qmlRegisterType<KSysGuard::FaceLoader>(uri, 1, 0, "FaceLoader");
}
