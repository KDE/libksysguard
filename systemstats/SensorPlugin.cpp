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

#include "SensorPlugin.h"

#include <sensors/sensors.h>

using namespace KSysGuard;

class Q_DECL_HIDDEN SensorPlugin::Private
{
public:
    QList<SensorContainer *> containers;
};

SensorPlugin::SensorPlugin(QObject *parent, const QVariantList &args)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    Q_UNUSED(args)
}

SensorPlugin::~SensorPlugin() = default;

QList<SensorContainer *> SensorPlugin::containers() const
{
    return d->containers;
}

QString SensorPlugin::providerName() const
{
    return QString();
}

void SensorPlugin::update()
{
}

void SensorPlugin::addContainer(SensorContainer *container)
{
    d->containers << container;
}

bool KSysGuard::SensorPlugin::initLibSensors()
{
    static bool inited = false;
    if (!inited) {
        inited = sensors_init(nullptr) == 0;
    }
    return inited;
}

