/*
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

#pragma once

#include <QQuickItem>
#include <QStandardItemModel>
#include <QJsonArray>
#include <KLocalizedContext>
#include <KPackage/PackageLoader>
#include <KConfigGroup>
#include <KConfigLoader>
#include <KDeclarative/ConfigPropertyMap>

#include "sensorfaces_export.h"

class QQmlEngine;

namespace KSysGuard {

class SensorFaceController;
class SensorFace;

class FacesModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum AdditionalRoles {
        PluginIdRole = Qt::UserRole + 1,
    };
    Q_ENUM(AdditionalRoles)

    FacesModel(QObject *parent = nullptr);
    ~FacesModel() = default;

    Q_INVOKABLE void reload();
    Q_INVOKABLE QString pluginId(int row);

    QHash<int, QByteArray> roleNames() const override;
};

class PresetsModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum AdditionalRoles {
        PluginIdRole = Qt::UserRole + 1,
        ConfigRole,
        WritableRole
    };
    Q_ENUM(AdditionalRoles)

    PresetsModel(QObject *parent = nullptr);
    ~PresetsModel() = default;

    Q_INVOKABLE void reload();

    QHash<int, QByteArray> roleNames() const override;
};

// This is exported so we can use it in autotests
class SENSORFACES_EXPORT SensorFaceControllerPrivate
{
public:
    SensorFaceControllerPrivate();

    QJsonArray readSensors(const KConfigGroup &config, const QString &entryName);
    QJsonArray readAndUpdateSensors(KConfigGroup &config, const QString &entryName);
    QJsonArray resolveSensors(const QJsonArray &partialEntries);
    SensorFace *createGui(const QString &qmlPath);
    QQuickItem *createConfigUi(const QString &file, const QVariantMap &initialProperties);

    SensorFaceController *q;
    QString title;
    QQmlEngine *engine;

    KConfigGroup faceProperties;
    KDeclarative::ConfigPropertyMap *faceConfiguration = nullptr;
    KConfigLoader *faceConfigLoader = nullptr;

    bool configNeedsSave = false;
    KPackage::Package facePackage;
    QString faceId;
    KLocalizedContext *contextObj = nullptr;
    KConfigGroup configGroup;
    KConfigGroup appearanceGroup;
    KConfigGroup sensorsGroup;
    KConfigGroup colorsGroup;
    QPointer <SensorFace> fullRepresentation;
    QPointer <SensorFace> compactRepresentation;
    QPointer <QQuickItem> faceConfigUi;
    QPointer <QQuickItem> appearanceConfigUi;
    QPointer <QQuickItem> sensorsConfigUi;

    QJsonArray totalSensors;
    QJsonArray highPrioritySensorIds;
    QJsonArray lowPrioritySensorIds;

    QTimer *syncTimer;
    bool shouldSync = true;
    FacesModel *availableFacesModel = nullptr;
    PresetsModel *availablePresetsModel = nullptr;

    static QVector<QPair<QRegularExpression, QString>> sensorIdReplacements;
};

}
