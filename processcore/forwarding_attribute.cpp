/*
 * Copyright 2021  David Redondo <kde@david-redondo.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "forwarding_attribute.h"

#include "process.h"

using namespace KSysGuard;

ForwardingAttribute::ForwardingAttribute(ProcessAttribute *actualAttribute, QObject *parent)
    : ProcessAttribute(actualAttribute->id(), actualAttribute->name(), parent)
    , actualAttribute(actualAttribute)
{
    setShortName(actualAttribute->shortName());
    setDescription(actualAttribute->description());
    setMin(actualAttribute->min());
    setMax(actualAttribute->max());
    setUnit(actualAttribute->unit());
    setVisibleByDefault(actualAttribute->isVisibleByDefault());

    connect(actualAttribute, &ProcessAttribute::dataChanged, this, &ProcessAttribute::dataChanged);
    connect(actualAttribute, &ProcessAttribute::enabledChanged, this, &ProcessAttribute::setEnabled);
}

QVariant ForwardingAttribute::data(quint64 pid) const
{
    return actualAttribute->data(pid);
}

QVariant ForwardingAttribute::cgroupData(KSysGuard::CGroup* cgroup) const
{
    return actualAttribute->cgroupData(cgroup);
}
