/*
    Copyright (C) 2019 Vlad Zahorodnii <vladzzag@gmail.com>
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

#include <QObject>
#include <QQmlParserStatus>
#include <QString>
#include <QVariant>

#include "formatter/Unit.h"

#include "sensors_export.h"

namespace KSysGuard
{

class SensorData;
class SensorInfo;

/**
 * An object encapsulating a backend sensor.
 *
 * This class represents a sensor as exposed by the backend. It allows querying
 * various metadata properties of the sensor as well as the current value.
 */
class SENSORS_EXPORT Sensor : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    /**
     * The path to the backend sensor this Sensor represents.
     */
    Q_PROPERTY(QString sensorId READ sensorId WRITE setSensorId NOTIFY sensorIdChanged)
    /**
     * The user-visible name of this Sensor.
     */
    Q_PROPERTY(QString name READ name NOTIFY metaDataChanged)
    /**
     * A shortened name that can be displayed when space is constrained.
     *
     * The value is the same as name if shortName was not provided by the backend.
     */
    Q_PROPERTY(QString shortName READ shortName NOTIFY metaDataChanged)
    /**
     * A description of the Sensor.
     */
    Q_PROPERTY(QString description READ description NOTIFY metaDataChanged)
    /**
     * The unit of this Sensor.
     */
    Q_PROPERTY(KSysGuard::Unit unit READ unit NOTIFY metaDataChanged)
    /**
     * The minimum value this Sensor can have.
     */
    Q_PROPERTY(qreal minimum READ minimum NOTIFY metaDataChanged)
    /**
     * The maximum value this Sensor can have.
     */
    Q_PROPERTY(qreal maximum READ maximum NOTIFY metaDataChanged)
    /**
     * The QVariant type for this sensor.
     *
     * This is used to create proper default values.
     */
    Q_PROPERTY(QVariant::Type type READ type NOTIFY metaDataChanged)
    /**
     * The status of the sensor.
     *
     * Due to the asynchronous nature of the underlying code, sensors are not
     * immediately available on construction. Instead, they need to request data
     * from the daemon and wait for it to arrive. This property reflects where
     * in that process this sensor is.
     */
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    /**
     * The current value of this sensor.
     */
    Q_PROPERTY(QVariant value READ value NOTIFY valueChanged)
    /**
     * A formatted version of \property value.
     */
    Q_PROPERTY(QString formattedValue READ formattedValue NOTIFY valueChanged)
    /**
     * Should this Sensor check for changes?
     *
     * Note that if set to true, the sensor will only be enabled when the parent
     * is also enabled.
     */
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    /**
     * This enum type is used to specify status of the Sensor.
     */
    enum class Status {
        Unknown, ///< The sensor has no ID assigned.
        Loading, ///< The sensor is currently being loaded.
        Ready, ///< The sensor has been loaded.
        Error, ///< An error occurred or the sensor has been removed.
        Removed, ///< Removed from backend
    };
    Q_ENUM(Status)

    explicit Sensor(QObject *parent = nullptr);
    explicit Sensor(const QString &id, QObject *parent = nullptr);
    ~Sensor() override;

    bool event(QEvent *event) override;

    QString sensorId() const;
    void setSensorId(const QString &id);
    Q_SIGNAL void sensorIdChanged() const;

    Status status() const;
    Q_SIGNAL void statusChanged() const;

    QString name() const;
    QString shortName() const;
    QString description() const;
    KSysGuard::Unit unit() const;
    qreal minimum() const;
    qreal maximum() const;
    QVariant::Type type() const;
    /**
     * This signal is emitted when any of the metadata properties change.
     */
    Q_SIGNAL void metaDataChanged() const;

    /**
     * Returns the output of the sensor.
     *
     * The returned value is the most recent sensor data received from the ksysguard
     * daemon, it's not necessarily the actual current output value.
     *
     * The client can't control how often the sensor data is sampled. The ksysguard
     * daemon is in charge of picking the sample rate. When the Sensor receives new
     * output value, dataChanged signal will be emitted.
     *
     * @see dataChanged
     */
    QVariant value() const;
    QString formattedValue() const;
    Q_SIGNAL void valueChanged() const;

    bool enabled() const;
    void setEnabled(bool newEnabled);
    Q_SIGNAL void enabledChanged();

    void classBegin() override;
    void componentComplete() override;

private:
    void onMetaDataChanged(const QString &sensorId, const SensorInfo &metaData);
    void onValueChanged(const QString &sensorId, const QVariant &value);
    void onEnabledChanged();

    class Private;
    const std::unique_ptr<Private> d;
};

} // namespace KSysGuard