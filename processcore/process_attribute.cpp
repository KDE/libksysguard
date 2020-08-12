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

#include "process_attribute.h"
#include "processes.h"
#include "cgroup.h"

using namespace KSysGuard;

class Q_DECL_HIDDEN KSysGuard::ProcessAttribute::Private
{
public:
    QString m_id;

    QString m_name;
    QString m_shortName;
    QString m_description;
    qreal m_min = 0;
    qreal m_max = 0;
    KSysGuard::Unit m_unit = KSysGuard::UnitInvalid; //Both a format hint and implies data type (i.e double/string)

    QHash<KSysGuard::Process *, QVariant> m_data;
    bool m_enabled = false;

    bool m_defaultVisible = false;
};

ProcessAttribute::ProcessAttribute(const QString &id, QObject *parent)
    : ProcessAttribute(id, QString(), parent)
{
}

ProcessAttribute::ProcessAttribute(const QString &id, const QString &name, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->m_id = id;
    d->m_name = name;
}

ProcessAttribute::~ProcessAttribute()
{
}

QString ProcessAttribute::id() const
{
    return d->m_id;
}

bool ProcessAttribute::enabled() const
{
    return d->m_enabled;
}

void ProcessAttribute::setEnabled(const bool enabled)
{
    if (d->m_enabled == enabled) {
        return;
    }
    d->m_enabled = enabled;
    emit enabledChanged(enabled);
}

QString ProcessAttribute::name() const
{
    return d->m_name;
}

void ProcessAttribute::setName(const QString &name)
{
    d->m_name = name;
}

QString ProcessAttribute::shortName() const
{
    return d->m_shortName.isEmpty() ? d->m_name : d->m_shortName;
}

void ProcessAttribute::setShortName(const QString &name)
{
    d->m_shortName = name;
}

QString ProcessAttribute::description() const
{
    return d->m_description;
}

void ProcessAttribute::setDescription(const QString &description)
{
    d->m_description = description;
}

qreal ProcessAttribute::min() const
{
    return d->m_min;
}

void ProcessAttribute::setMin(const qreal min)
{
    d->m_min = min;
}

qreal ProcessAttribute::max() const
{
    return d->m_max;
}

void ProcessAttribute::setMax(const qreal max)
{
    d->m_max = max;
}

KSysGuard::Unit ProcessAttribute::unit() const
{
    return d->m_unit;
}

void ProcessAttribute::setUnit(KSysGuard::Unit unit)
{
    d->m_unit = unit;
}

bool KSysGuard::ProcessAttribute::isVisibleByDefault() const
{
    return d->m_defaultVisible;
}

void KSysGuard::ProcessAttribute::setVisibleByDefault(bool visible)
{
    d->m_defaultVisible = visible;
}

QVariant ProcessAttribute::data(KSysGuard::Process *process) const
{
    return d->m_data.value(process);
}

void ProcessAttribute::setData(KSysGuard::Process *process, const QVariant &value)
{
    d->m_data[process] = value;
    emit dataChanged(process);
}

void ProcessAttribute::clearData(KSysGuard::Process *process)
{
    d->m_data.remove(process);
    emit dataChanged(process);
}

QVariant ProcessAttribute::cgroupData(KSysGuard::CGroup *cgroup, const QVector<KSysGuard::Process*> &groupProcesses) const
{
    Q_UNUSED(cgroup)

    if (groupProcesses.isEmpty()) {
        return QVariant{};
    }

    qreal total = std::accumulate(groupProcesses.constBegin(), groupProcesses.constEnd(), 0.0, [this](qreal total, KSysGuard::Process *process) {
        return total + data(process).toDouble();
    });
    return QVariant(total);
}
