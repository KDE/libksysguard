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

#include "SensorDaemonInterface_p.h"

#include <QDBusPendingCallWatcher>

#include "ksysguarddaemon.h"

using namespace KSysGuard;

class SensorDaemonInterface::Private
{
public:
    std::unique_ptr<org::kde::KSysGuardDaemon> dbusInterface;
    std::unique_ptr<QDBusServiceWatcher> serviceWatcher;
    QStringList subscribedSensors;
    static const QString SensorServiceName;
    static const QString SensorPath;
};

const QString SensorDaemonInterface::Private::SensorServiceName = QStringLiteral("org.kde.ksystemstats");
const QString SensorDaemonInterface::Private::SensorPath = QStringLiteral("/");

SensorDaemonInterface::SensorDaemonInterface(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    qDBusRegisterMetaType<SensorData>();
    qDBusRegisterMetaType<SensorInfo>();
    qDBusRegisterMetaType<SensorDataList>();
    qDBusRegisterMetaType<SensorInfoMap>();

    d->serviceWatcher = std::make_unique<QDBusServiceWatcher>(d->SensorServiceName, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForUnregistration);
    connect(d->serviceWatcher.get(), &QDBusServiceWatcher::serviceUnregistered, this, &SensorDaemonInterface::reconnect);
    reconnect();
}

void KSysGuard::SensorDaemonInterface::reconnect()
{
    d->dbusInterface = std::make_unique<org::kde::KSysGuardDaemon>(Private::SensorServiceName, Private::SensorPath, QDBusConnection::sessionBus());
    connect(d->dbusInterface.get(), &org::kde::KSysGuardDaemon::sensorMetaDataChanged, this, &SensorDaemonInterface::onMetaDataChanged);
    connect(d->dbusInterface.get(), &org::kde::KSysGuardDaemon::newSensorData, this, &SensorDaemonInterface::onValueChanged);
    connect(d->dbusInterface.get(), &org::kde::KSysGuardDaemon::sensorAdded, this, &SensorDaemonInterface::sensorAdded);
    connect(d->dbusInterface.get(), &org::kde::KSysGuardDaemon::sensorRemoved, this, &SensorDaemonInterface::sensorRemoved);
    subscribe(d->subscribedSensors);
}

SensorDaemonInterface::~SensorDaemonInterface()
{
}

void SensorDaemonInterface::requestMetaData(const QString &sensorId)
{
    requestMetaData(QStringList{sensorId});
}

void SensorDaemonInterface::requestMetaData(const QStringList &sensorIds)
{
    auto watcher = new QDBusPendingCallWatcher{d->dbusInterface->sensors(sensorIds), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, watcher, [this](QDBusPendingCallWatcher *self) {
        self->deleteLater();

        const QDBusPendingReply<SensorInfoMap> reply = *self;
        if (reply.isError()) {
            return;
        }

        const auto infos = reply.value();
        for (auto itr = infos.begin(); itr != infos.end(); ++itr) {
            Q_EMIT metaDataChanged(itr.key(), itr.value());
        }
    });
}

void SensorDaemonInterface::requestValue(const QString &sensorId)
{
    auto watcher = new QDBusPendingCallWatcher{d->dbusInterface->sensorData({sensorId}), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, watcher, [this](QDBusPendingCallWatcher *self) {
        self->deleteLater();

        const QDBusPendingReply<SensorDataList> reply = *self;
        if (reply.isError()) {
            return;
        }

        const auto allData = reply.value();
        for (auto data : allData) {
            Q_EMIT valueChanged(data.sensorProperty, data.payload);
        }
    });
}

QDBusPendingCallWatcher *SensorDaemonInterface::allSensors() const
{
    return new QDBusPendingCallWatcher{d->dbusInterface->allSensors()};
}

void SensorDaemonInterface::subscribe(const QString &sensorId)
{
    subscribe(QStringList{sensorId});
}

void KSysGuard::SensorDaemonInterface::subscribe(const QStringList &sensorIds)
{
    d->dbusInterface->subscribe(sensorIds);
    d->subscribedSensors.append(sensorIds);
}

void SensorDaemonInterface::unsubscribe(const QString &sensorId)
{
    unsubscribe(QStringList{sensorId});
}

void KSysGuard::SensorDaemonInterface::unsubscribe(const QStringList &sensorIds)
{
    d->dbusInterface->unsubscribe(sensorIds);
}

SensorDaemonInterface *SensorDaemonInterface::instance()
{
    static SensorDaemonInterface instance;
    return &instance;
}

void SensorDaemonInterface::onMetaDataChanged(const QHash<QString, SensorInfo> &metaData)
{
    for (auto itr = metaData.begin(); itr != metaData.end(); ++itr) {
        Q_EMIT metaDataChanged(itr.key(), itr.value());
    }
}

void SensorDaemonInterface::onValueChanged(const SensorDataList &values)
{
    for (auto entry : values) {
        Q_EMIT valueChanged(entry.sensorProperty, entry.payload);
    }
}
