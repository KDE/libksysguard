/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KConfigGroup>
#include <KConfigLoader>
#include <KDeclarative/ConfigPropertyMap>
#include <KLocalizedContext>
#include <KPackage/PackageLoader>
#include <QJsonArray>
#include <QQuickItem>
#include <QStandardItemModel>

#include <functional>

#include "sensorfaces_export.h"

class QQmlEngine;

namespace KSysGuard
{
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
    enum AdditionalRoles { PluginIdRole = Qt::UserRole + 1, ConfigRole, WritableRole };
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
    QString replaceDiskId(const QString &entryname) const;
    QString replacePartitionId(const QString &entryname) const;
    void resolveSensors(const QJsonArray &partialEntries, std::function<void(const QJsonArray &)> callback);
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
    KConfigGroup labelsGroup;
    QPointer<SensorFace> fullRepresentation;
    QPointer<SensorFace> compactRepresentation;
    QPointer<QQuickItem> faceConfigUi;
    QPointer<QQuickItem> appearanceConfigUi;
    QPointer<QQuickItem> sensorsConfigUi;

    QJsonArray totalSensors;
    QJsonArray highPrioritySensorIds;
    QJsonArray lowPrioritySensorIds;

    QTimer *syncTimer;
    bool shouldSync = true;
    FacesModel *availableFacesModel = nullptr;
    PresetsModel *availablePresetsModel = nullptr;

    static QVector<QPair<QRegularExpression, QString>> sensorIdReplacements;
    static QRegularExpression oldDiskSensor;
    static QRegularExpression oldPartitionSensor;
};

}
