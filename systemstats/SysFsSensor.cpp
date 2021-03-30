/*
    Copyright (c) 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

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

#include "SysFsSensor.h"

#include <QFile>

using namespace KSysGuard;

class Q_DECL_HIDDEN SysFsSensor::Private
{
public:
    QString path;
    std::function<QVariant(const QByteArray&)> convertFunction;
};

SysFsSensor::SysFsSensor(const QString& id, const QString& path, SensorObject* parent)
    : SensorProperty(id, parent)
    , d(std::make_unique<Private>())
{
    d->path = path;

    d->convertFunction = [](const QByteArray &input) {
        return std::atoll(input);
    };
}

SysFsSensor::~SysFsSensor() = default;

void SysFsSensor::setConvertFunction(const std::function<QVariant (const QByteArray &)>& function)
{
    d->convertFunction = function;
}

void SysFsSensor::update()
{
    if (!isSubscribed()) {
        return;
    }

    QFile file(d->path);
    if (!file.exists()) {
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    auto value = file.readAll();
    setValue(d->convertFunction(value));
}
