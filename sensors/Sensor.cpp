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

#include <QEvent>

#include "Sensor.h"
#include "SensorDaemonInterface_p.h"
#include "SensorInfo_p.h"
#include "formatter/Formatter.h"
#include "SensorQuery.h"

using namespace KSysGuard;

class Q_DECL_HIDDEN Sensor::Private
{
public:
    SensorInfo sensorInfo;

    Sensor::Status status = Sensor::Status::Unknown;
    QVariant value;

    bool usedByQml = false;
    bool componentComplete = false;

    QString pendingId;
    QString id;

    bool enabled = true;
};

Sensor::Sensor(QObject *parent)
    : Sensor(QString{}, parent)
{
}

Sensor::Sensor(const QString &id, QObject *parent)
    : QObject(parent)
    , d(new Private())
{
    connect(this, &Sensor::statusChanged, this, &Sensor::valueChanged);
    connect(this, &Sensor::statusChanged, this, &Sensor::metaDataChanged);
    connect(this, &Sensor::enabledChanged, this, &Sensor::onEnabledChanged);

    setSensorId(id);
}

Sensor::Sensor(const SensorQuery &query, int index, QObject *parent)
    : Sensor(QString{}, parent)
{
    if (index >= 0 && index < query.result().size()) {
        auto result = query.result().at(index);
        d->id = result.first;
        onMetaDataChanged(d->id, result.second);
        onEnabledChanged();
    }
}

bool Sensor::event(QEvent *event)
{
    if (event->type() == QEvent::ParentAboutToChange && parent()) {
        parent()->disconnect(this);
    } else if (event->type() == QEvent::ParentChange && parent()) {
        if (parent()->metaObject()->indexOfSignal("enabledChanged()") != -1) {
            connect(parent(), SIGNAL(enabledChanged()), this, SIGNAL(enabledChanged()));
        }
    }

    return QObject::event(event);
}

Sensor::~Sensor()
{
    SensorDaemonInterface::instance()->unsubscribe(d->id);
}

QString Sensor::sensorId() const
{
    return d->id;
}

void Sensor::setSensorId(const QString &id)
{
    if (id == d->id) {
        return;
    }

    if (d->usedByQml && !d->componentComplete) {
        d->pendingId = id;
        return;
    }

    d->id = id;
    d->status = Sensor::Status::Loading;

    if (!id.isEmpty()) {
        SensorDaemonInterface::instance()->requestMetaData(id);
        connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::metaDataChanged, this, &Sensor::onMetaDataChanged, Qt::UniqueConnection);
    }

    if (enabled()) {
        SensorDaemonInterface::instance()->subscribe(id);
        SensorDaemonInterface::instance()->requestValue(id);
        connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::valueChanged, this, &Sensor::onValueChanged, Qt::UniqueConnection);
    }

    Q_EMIT sensorIdChanged();
    Q_EMIT statusChanged();
}

Sensor::Status Sensor::status() const
{
    return d->status;
}

QString Sensor::name() const
{
    return d->sensorInfo.name;
}

QString Sensor::shortName() const
{
    if (d->sensorInfo.shortName.isEmpty()) {
        return d->sensorInfo.name;
    }

    return d->sensorInfo.shortName;
}

QString Sensor::description() const
{
    return d->sensorInfo.description;
}

Unit Sensor::unit() const
{
    return d->sensorInfo.unit;
}

qreal Sensor::minimum() const
{
    return d->sensorInfo.min;
}

qreal Sensor::maximum() const
{
    return d->sensorInfo.max;
}

QVariant::Type Sensor::type() const
{
    return d->sensorInfo.variantType;
}

QVariant Sensor::value() const
{
    if (!d->value.isValid()) {
        return QVariant{d->sensorInfo.variantType};
    }
    return d->value;
}

QString Sensor::formattedValue() const
{
    return Formatter::formatValue(value(), unit(), MetricPrefixAutoAdjust, FormatOptionShowNull);
}

bool Sensor::enabled() const
{
    if (d->enabled && parent()) {
        auto parentEnabled = parent()->property("enabled");
        if (parentEnabled.isValid()) {
            return parentEnabled.toBool();
        }
    }

    return d->enabled;
}

void Sensor::setEnabled(bool newEnabled)
{
    if (newEnabled == d->enabled) {
        return;
    }

    d->enabled = newEnabled;
    Q_EMIT enabledChanged();
}

void Sensor::classBegin()
{
    d->usedByQml = true;
}

void Sensor::componentComplete()
{
    d->componentComplete = true;

    setSensorId(d->pendingId);

    if (parent() && parent()->metaObject()->indexOfSignal("enabledChanged()") != -1) {
        connect(parent(), SIGNAL(enabledChanged()), this, SIGNAL(enabledChanged()));
    }
}

void Sensor::onMetaDataChanged(const QString &sensorId, const SensorInfo &metaData)
{
    if (sensorId != d->id || !enabled()) {
        return;
    }

    d->sensorInfo = metaData;

    if (d->status == Sensor::Status::Loading) {
        d->status = Sensor::Status::Ready;
        Q_EMIT statusChanged();
    }

    Q_EMIT metaDataChanged();
}

void Sensor::onValueChanged(const QString &sensorId, const QVariant &value)
{
    if (sensorId != d->id || !enabled()) {
        return;
    }

    d->value = value;
    Q_EMIT valueChanged();
}

void Sensor::onEnabledChanged()
{
    if (enabled()) {
        connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::metaDataChanged, this, &Sensor::onMetaDataChanged, Qt::UniqueConnection);
        connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::valueChanged, this, &Sensor::onValueChanged, Qt::UniqueConnection);

        SensorDaemonInterface::instance()->subscribe(d->id);
        // Force an update of metadata and data, since that may have changed
        // while we were disabled.
        SensorDaemonInterface::instance()->requestMetaData(d->id);
        SensorDaemonInterface::instance()->requestValue(d->id);
    } else {
        disconnect(SensorDaemonInterface::instance(), &SensorDaemonInterface::metaDataChanged, this, &Sensor::onMetaDataChanged);
        disconnect(SensorDaemonInterface::instance(), &SensorDaemonInterface::valueChanged, this, &Sensor::onValueChanged);
        SensorDaemonInterface::instance()->unsubscribe(d->id);
    }
}
