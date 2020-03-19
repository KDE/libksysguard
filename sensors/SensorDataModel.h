/*
    Copyright (c) 2019 Eike Hein <hein@kde.org>
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

#pragma once

#include <memory>
#include <QAbstractTableModel>
#include <QQmlParserStatus>
#include <QDateTime>
#include "sensors_export.h"

namespace KSysGuard
{

class SensorInfo;

class SENSORS_EXPORT SensorDataModel : public QAbstractTableModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QStringList sensors READ sensors WRITE setSensors NOTIFY sensorsChanged)
    // The minimum value of all sensors' minimum property.
    Q_PROPERTY(qreal minimum READ minimum NOTIFY sensorMetaDataChanged)
    // The maximum value of all sensors' maximum property.
    Q_PROPERTY(qreal maximum READ maximum NOTIFY sensorMetaDataChanged)

public:
    enum AdditionalRoles {
        SensorId = Qt::UserRole + 1,
        Name,
        ShortName,
        Description,
        Unit,
        Minimum,
        Maximum,
        Type,
        Value,
        FormattedValue,
    };
    Q_ENUM(AdditionalRoles)

    explicit SensorDataModel(const QStringList &sensorIds = {}, QObject *parent = nullptr);
    virtual ~SensorDataModel();

    QHash<int, QByteArray> roleNames() const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QStringList sensors() const;
    void setSensors(const QStringList &sensorIds);
    Q_SIGNAL void sensorsChanged() const;
    Q_SIGNAL void sensorMetaDataChanged();

    qreal minimum() const;
    qreal maximum() const;

    Q_INVOKABLE void addSensor(const QString &sensorId);
    Q_INVOKABLE void removeSensor(const QString &sensorId);
    Q_INVOKABLE int column(const QString &sensorId) const;

    void classBegin() override;
    void componentComplete() override;

private:
    void onSensorAdded(const QString &sensorId);
    void onSensorRemoved(const QString &sensorId);
    void onMetaDataChanged(const QString &sensorId, const SensorInfo &info);
    void onValueChanged(const QString &sensorId, const QVariant &value);

    class Private;
    const std::unique_ptr<Private> d;
};

} // namespace KSysGuard
