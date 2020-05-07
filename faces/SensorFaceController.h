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

#include <KConfigGroup>

#include "sensorfaces_export.h"

namespace KDeclarative {
    class ConfigPropertyMap;
}

class QQmlEngine;
class KDesktopFile;
class KConfigLoader;

namespace KSysGuard {

class SensorFace;

class SENSORFACES_EXPORT SensorFaceController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString faceId READ faceId WRITE setFaceId NOTIFY faceIdChanged)
    Q_PROPERTY(QJsonArray totalSensors READ totalSensors WRITE setTotalSensors NOTIFY totalSensorsChanged)
    Q_PROPERTY(QJsonArray highPrioritySensorIds READ highPrioritySensorIds WRITE setHighPrioritySensorIds NOTIFY highPrioritySensorIdsChanged)
    Q_PROPERTY(QJsonArray highPrioritySensorColors READ highPrioritySensorColors WRITE setHighPrioritySensorColors NOTIFY highPrioritySensorColorsChanged)
    Q_PROPERTY(QJsonArray lowPrioritySensorIds READ lowPrioritySensorIds WRITE setLowPrioritySensorIds NOTIFY lowPrioritySensorIdsChanged)

    Q_PROPERTY(QString name READ name NOTIFY faceIdChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY faceIdChanged)
    Q_PROPERTY(bool supportsSensorsColors READ supportsSensorsColors NOTIFY faceIdChanged)
    Q_PROPERTY(bool supportsTotalSensors READ supportsTotalSensors NOTIFY faceIdChanged)
    Q_PROPERTY(bool supportsLowPrioritySensors READ supportsLowPrioritySensors NOTIFY faceIdChanged)
    Q_PROPERTY(KDeclarative::ConfigPropertyMap *faceConfiguration READ faceConfiguration NOTIFY faceIdChanged)

    Q_PROPERTY(QQuickItem *fullRepresentation READ fullRepresentation NOTIFY faceIdChanged)
    Q_PROPERTY(QQuickItem *compactRepresentation READ compactRepresentation NOTIFY faceIdChanged)
    Q_PROPERTY(QQuickItem *faceConfigUi READ faceConfigUi NOTIFY faceIdChanged)
    Q_PROPERTY(QQuickItem *appearanceConfigUi READ appearanceConfigUi NOTIFY faceIdChanged)
    Q_PROPERTY(QQuickItem *sensorsConfigUi READ sensorsConfigUi NOTIFY faceIdChanged)

    Q_PROPERTY(QAbstractItemModel *availableFacesModel READ availableFacesModel CONSTANT)
    Q_PROPERTY(QAbstractItemModel *availablePresetsModel READ availablePresetsModel CONSTANT)

public:
    SensorFaceController(KConfigGroup &config, QQmlEngine *engine);
    ~SensorFaceController();

    void setFaceId(const QString &face);
    QString faceId() const;

    QQuickItem *fullRepresentation();
    QQuickItem *compactRepresentation();
    QQuickItem *faceConfigUi();
    QQuickItem *appearanceConfigUi();
    QQuickItem *sensorsConfigUi();

    KDeclarative::ConfigPropertyMap *faceConfiguration() const;

    QString title() const;
    void setTitle(const QString &title);

    QJsonArray totalSensors() const;
    void setTotalSensors(const QJsonArray &sensor);

    QJsonArray highPrioritySensorIds() const;
    void setHighPrioritySensorIds(const QJsonArray &ids);

    QJsonArray sensors() const;

    QJsonArray highPrioritySensorColors() const;
    void setHighPrioritySensorColors(const QJsonArray &colors);

    QJsonArray lowPrioritySensorIds() const;
    void setLowPrioritySensorIds(const QJsonArray &ids);

    // from face config, immutable by the user
    QString name() const;
    const QString icon() const;

    bool supportsSensorsColors() const;
    bool supportsTotalSensors() const;
    bool supportsLowPrioritySensors() const;

    QAbstractItemModel *availableFacesModel();
    QAbstractItemModel *availablePresetsModel();

    Q_INVOKABLE void reloadConfig();
    Q_INVOKABLE void loadPreset(const QString &preset);
    Q_INVOKABLE void savePreset();
    Q_INVOKABLE void uninstallPreset(const QString &pluginId);

Q_SIGNALS:
    void faceIdChanged();
    void titleChanged();
    void totalSensorsChanged();
    void highPrioritySensorIdsChanged();
    void highPrioritySensorColorsChanged();
    void lowPrioritySensorIdsChanged();
    void sensorsChanged();

private:
    class Private;
    const std::unique_ptr<Private> d;
};
}
