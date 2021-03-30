/*
    Copyright (c) 2019 David Edmundson <davidedmundson@kde.org>

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

#include "SensorProperty.h"
#include "SensorObject.h"

using namespace KSysGuard;

class Q_DECL_HIDDEN SensorProperty::Private
{
public:
    SensorObject *parent = nullptr;
    SensorInfo info;
    QString id;
    QString name;
    QString prefix;
    QVariant value;
    int subscribers = 0;
};


SensorProperty::SensorProperty(const QString &id, SensorObject *parent)
    : SensorProperty(id, QString(), parent)
{
}

SensorProperty::SensorProperty(const QString &id, const QString &name, SensorObject *parent)
    : SensorProperty(id, name, QVariant(), parent)
{
}

SensorProperty::SensorProperty(const QString &id, const QString &name, const QVariant &initalValue, SensorObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->id = id;
    d->parent = parent;
    setName(name);
    if (initalValue.isValid()) {
        setValue(initalValue);
    }
    parent->addProperty(this);
}

SensorProperty::~SensorProperty()
{
}

SensorInfo SensorProperty::info() const
{
    return d->info;
}

QString SensorProperty::id() const
{
    return d->id;
}

QString SensorProperty::path() const
{
    return d->parent->path() % QLatin1Char('/') % d->id;
}

void SensorProperty::setName(const QString &name)
{
    if (d->name == name) {
        return;
    }

    d->name = name;
    d->info.name = d->prefix.isEmpty() ? d->name : d->prefix % QLatin1Char(' ') % d->name;
    emit sensorInfoChanged();
}

void SensorProperty::setShortName(const QString &name)
{
    if (d->info.shortName == name) {
        return;
    }

    d->info.shortName = name;
    emit sensorInfoChanged();
}

void SensorProperty::setPrefix(const QString &prefix)
{
    if (d->prefix == prefix) {
        return;
    }

    d->prefix = prefix;
    d->info.name = prefix.isEmpty() ? d->name : prefix % QLatin1Char(' ') % d->name;
    emit sensorInfoChanged();
}

void SensorProperty::setDescription(const QString &description)
{
    if (d->info.description == description) {
        return;
    }

    d->info.description = description;
    emit sensorInfoChanged();
}

void SensorProperty::setMin(qreal min)
{
    if (qFuzzyCompare(d->info.min, min)) {
        return;
    }

    d->info.min = min;
    emit sensorInfoChanged();
}

void SensorProperty::setMax(qreal max)
{
    if (qFuzzyCompare(d->info.max, max)) {
        return;
    }

    d->info.max = max;
    emit sensorInfoChanged();
}

void SensorProperty::setMax(SensorProperty *other)
{
    setMax(other->value().toReal());
    if (isSubscribed()) {
        other->subscribe();
    }
    connect(this, &SensorProperty::subscribedChanged, this, [this, other](bool isSubscribed) {
        if (isSubscribed) {
            other->subscribe();
            setMax(other->value().toReal());
        } else {
            other->unsubscribe();
        }
    });
    connect(other, &SensorProperty::valueChanged, this, [this, other]() {
        setMax(other->value().toReal());
    });
}

void SensorProperty::setUnit(KSysGuard::Unit unit)
{
    if (d->info.unit == unit) {
        return;
    }

    d->info.unit = unit;
    emit sensorInfoChanged();
}

void SensorProperty::setVariantType(QVariant::Type type)
{
    if (d->info.variantType == type) {
        return;
    }

    d->info.variantType = type;
    emit sensorInfoChanged();
}

bool SensorProperty::isSubscribed() const
{
    return d->subscribers > 0;
}

void SensorProperty::subscribe()
{
    d->subscribers++;
    if (d->subscribers == 1) {
        emit subscribedChanged(true);
    }
}

void SensorProperty::unsubscribe()
{
    d->subscribers--;
    if (d->subscribers == 0) {
        emit subscribedChanged(false);
    }
}

QVariant SensorProperty::value() const
{
    return d->value;
}

void SensorProperty::setValue(const QVariant &value)
{
    d->value = value;
    emit valueChanged();
}
