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
#include "sensors_export.h"

namespace KSysGuard {

class SensorInfo;

class SENSORS_EXPORT SensorQuery : public QObject
{
    Q_OBJECT

public:
    SensorQuery(const QString &path = QString{}, QObject *parent = nullptr);
    ~SensorQuery() override;

    QString path() const;
    void setPath(const QString &path);

    QStringList sensorIds() const;
    QVector<QPair<QString, SensorInfo>> result() const;

    bool execute();
    bool waitForFinished();

    Q_SIGNAL void finished(const SensorQuery *query);

private:
    class Private;
    const std::unique_ptr<Private> d;
};

} // namespace KSysGuard
