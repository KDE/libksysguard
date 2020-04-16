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

#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QAbstractItemModel>

#include <KPackage/Package>
#include <KConfigGroup>
#include <KConfigLoader>

#include "sensors_export.h"

namespace KDeclarative {
    class ConfigPropertyMap;
}

class QQmlEngine;
class KDesktopFile;
class KConfigLoader;
class SensorFace;

class SENSORS_EXPORT SensorFaceController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString faceId READ faceId WRITE setFaceId NOTIFY faceIdChanged)
    Q_PROPERTY(QString totalSensor READ totalSensor WRITE setTotalSensor NOTIFY totalSensorChanged)
    Q_PROPERTY(QStringList sensorIds READ sensorIds WRITE setSensorIds NOTIFY sensorIdsChanged)
    Q_PROPERTY(QStringList sensorColors READ sensorColors WRITE setSensorColors NOTIFY sensorColorsChanged)
    Q_PROPERTY(QStringList textOnlySensorIds READ textOnlySensorIds WRITE setTextOnlySensorIds NOTIFY textOnlySensorIdsChanged)


    Q_PROPERTY(QString name READ name NOTIFY faceIdChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY faceIdChanged)
    Q_PROPERTY(bool supportsSensorsColors READ supportsSensorsColors NOTIFY faceIdChanged)
    Q_PROPERTY(bool supportsTotalSensor READ supportsTotalSensor NOTIFY faceIdChanged)
    Q_PROPERTY(bool supportsTextOnlySensors READ supportsTextOnlySensors NOTIFY faceIdChanged)
    Q_PROPERTY(KDeclarative::ConfigPropertyMap *faceConfiguration READ faceConfiguration NOTIFY faceIdChanged)

    Q_PROPERTY(SensorFace *fullRepresentation READ fullRepresentation NOTIFY faceIdChanged)
    Q_PROPERTY(SensorFace *compactRepresentation READ compactRepresentation NOTIFY faceIdChanged)
    Q_PROPERTY(QQuickItem *faceConfigUi READ faceConfigUi NOTIFY faceIdChanged)

    Q_PROPERTY(QAbstractItemModel *availableFacesModel READ availableFacesModel CONSTANT)
    Q_PROPERTY(QAbstractItemModel *availablePresetsModel READ availablePresetsModel CONSTANT)

public:
    SensorFaceController(KConfigGroup &config, QQmlEngine *engine);
    ~SensorFaceController();

    void setFaceId(const QString &face);
    QString faceId() const;

    //TODO: just QQuickItem
    SensorFace *fullRepresentation();
    SensorFace *compactRepresentation();
    QQuickItem *faceConfigUi();

    KDeclarative::ConfigPropertyMap *faceConfiguration() const;

    QString title() const;
    void setTitle(const QString &title);

    QString totalSensor() const;
    void setTotalSensor(const QString &sensor);

    QStringList sensorIds() const;
    void setSensorIds(const QStringList &ids);

    QStringList sensorColors() const;
    void setSensorColors(const QStringList &colors);

    QStringList textOnlySensorIds() const;
    void setTextOnlySensorIds(const QStringList &ids);

    // from face config, immutable by the user
    QString name() const;
    const QString icon() const;

    bool supportsSensorsColors() const;
    bool supportsTotalSensor() const;
    bool supportsTextOnlySensors() const;

    QAbstractItemModel *availableFacesModel();
    QAbstractItemModel *availablePresetsModel();

    Q_INVOKABLE void loadPreset(const QString &preset);
    Q_INVOKABLE void savePreset();
    Q_INVOKABLE void uninstallPreset(const QString &pluginId);

Q_SIGNALS:
    void faceIdChanged();
    void titleChanged();
    void totalSensorChanged();
    void sensorIdsChanged();
    void sensorColorsChanged();
    void textOnlySensorIdsChanged();

private:
    class Private;
    const std::unique_ptr<Private> d;
};
