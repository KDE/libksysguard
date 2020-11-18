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

#include "sensors_export.h"
#include <QAbstractTableModel>
#include <QDateTime>
#include <QQmlParserStatus>
#include <memory>

namespace KSysGuard
{
class SensorInfo;

/**
 * A model representing a table of sensors.
 *
 * This model will expose the metadata and values of a list of sensors as a
 * table, using one column for each sensor. The metadata and values are
 * represented as different roles.
 */
class SENSORS_EXPORT SensorDataModel : public QAbstractTableModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    /**
     * The list of sensors to watch.
     */
    Q_PROPERTY(QStringList sensors READ sensors WRITE setSensors NOTIFY sensorsChanged)
    /**
     * The minimum value of all sensors' minimum property.
     */
    Q_PROPERTY(qreal minimum READ minimum NOTIFY sensorMetaDataChanged)
    /**
     * The maximum value of all sensors' maximum property.
     */
    Q_PROPERTY(qreal maximum READ maximum NOTIFY sensorMetaDataChanged)
    /**
     * Should this model be updated or not. Defaults to true.
     */
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

    /**
     * Used by the model to provide data for the Color role if set.
     */
    Q_PROPERTY(QVariantMap sensorColors READ sensorColors WRITE setSensorColors NOTIFY sensorColorsChanged)

public:
    /**
     * The roles that this model exposes
     * @see Sensor
     */
    enum AdditionalRoles {
        SensorId = Qt::UserRole + 1, ///< The backend path to the sensor.
        Name,                        ///< The name of the sensor.
        ShortName,                   ///< A shorter name for the sensor. This is equal to name if not set.
        Description,                 ///< A description for the sensor.
        Unit,                        ///< The unit of the sensor.
        Minimum,                     ///< The minimum value this sensor can have.
        Maximum,                     ///< The maximum value this sensor can have.
        Type,                        ///< The QVariant::Type of the sensor.
        Value,                       ///< The value of the sensor.
        FormattedValue,              ///< A formatted string of the value of the sensor.
        Color,                       ///< A color of the sensor, if sensorColors is set
        UpdateInterval,              ///< The time in milliseconds between each update of the sensor.
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

    bool enabled() const;
    void setEnabled(bool newEnabled);
    Q_SIGNAL void enabledChanged();

    qreal minimum() const;
    qreal maximum() const;

    QVariantMap sensorColors() const;
    void setSensorColors(const QVariantMap &sensorColors);
    Q_SIGNAL void sensorColorsChanged();

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
