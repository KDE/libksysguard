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

#pragma once

#include <memory>
#include <QObject>

#include "systemstats/SensorInfo.h"

class QDBusPendingCallWatcher;

namespace KSysGuard
{
/**
 * Internal helper class to communicate with the daemon.
 *
 * This is mostly for convenience on top of the auto-generated KSysGuardDaemon
 * D-Bus interface.
 */
class SensorDaemonInterface : public QObject
{
    Q_OBJECT

public:
    SensorDaemonInterface(QObject *parent = nullptr);
    ~SensorDaemonInterface() override;

    void requestMetaData(const QString &sensorId);
    void requestMetaData(const QStringList &sensorIds);
    Q_SIGNAL void metaDataChanged(const QString &sensorId, const SensorInfo &info);
    void requestValue(const QString &sensorId);
    Q_SIGNAL void valueChanged(const QString &sensorId, const QVariant &value);

    QDBusPendingCallWatcher *allSensors() const;

    void subscribe(const QString &sensorId);
    void subscribe(const QStringList &sensorIds);
    void unsubscribe(const QString &sensorId);
    void unsubscribe(const QStringList &sensorIds);

    Q_SIGNAL void sensorAdded(const QString &sensorId);
    Q_SIGNAL void sensorRemoved(const QString &sensorId);

    static SensorDaemonInterface *instance();

private:
    void onMetaDataChanged(const QHash<QString, SensorInfo> &metaData);
    void onValueChanged(const SensorDataList &values);
    void reconnect();

    class Private;
    const std::unique_ptr<Private> d;
};

}
