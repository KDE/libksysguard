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

/**
 * An object to query the daemon for a list of sensors and their metadata.
 *
 * This class will request a list of sensors from the daemon, then filter them
 * based on the supplied path. The path can include the wildcard "*" to get a
 * list of all sensors matching the specified part of their path. In addition,
 * if left empty, all sensors will be returned.
 */
class SENSORS_EXPORT SensorQuery : public QObject
{
    Q_OBJECT

public:
    SensorQuery(const QString &path = QString{}, QObject *parent = nullptr);
    ~SensorQuery() override;

    QString path() const;
    void setPath(const QString &path);

    QStringList sensorIds() const;
    /**
     * The result of the query.
     *
     * \return a Vector of sensor ID, SensorInfo pairs that match the given path
     * expression.
     */
    QVector<QPair<QString, SensorInfo>> result() const;

    /**
     * Start processing the query.
     */
    bool execute();
    /**
     * Wait for the query to finish.
     *
     * Mostly useful for code that needs the result to be available before
     * continuing. Ideally the finished() signal should be used instead.
     */
    bool waitForFinished();

    Q_SIGNAL void finished(const SensorQuery *query);

private:
    class Private;
    const std::unique_ptr<Private> d;
};

} // namespace KSysGuard
