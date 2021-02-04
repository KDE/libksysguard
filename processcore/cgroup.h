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

#pragma once

#include <QScopedPointer>
#include <QString>
#include <QVector>

#include <KService>

namespace KSysGuard
{

class Process;
class CGroupPrivate;

/**
 * @brief The CGroup class represents a cgroup. This could be a
 * service, slice or scope
 */
class Q_DECL_EXPORT CGroup
{
public:
    virtual ~CGroup();
    /**
     * @brief id
     * @return The cgroup ID passed from the constructor
     */
    QString id() const;

    /**
     * @brief Returns metadata about the given service
     * Only applicable for .service entries and really only useful for applications.
     * This KService object is always valid, but may not correspond to a real desktop entry
     * @return
     */
    KService::Ptr service() const;

    /**
     * @brief The list of pids contained in this group.
     * @return A Vector of pids
     */
    QVector<pid_t> pids() const;

    /**
     * Request fetching the list of processes associated with this cgroup.
     *
     * This is done in a separate thread. Once it has completed, \p callback is
     * called with the list of pids of this cgroup.
     *
     * \param context An object that is used to track if the caller still exists.
     * \param callback A callback that gets called once the list of pids has
     *                 been retrieved.
     */
    void requestPids(QPointer<QObject> context, std::function<void()> callback);

    /**
     * Returns the base path to exposed cgroup information. Either /sys/fs/cgroup or /sys/fs/cgroup/unified as applicable
     * If cgroups are unavailable this will be an empty string
     */
    static QString cgroupSysBasePath();

private:
    /**
     * Create a new cgroup object for a given cgroup entry
     * The id is the fully formed separated path, such as
     * "system.slice/dbus.service"
     */
    CGroup(const QString &id);

    QScopedPointer<CGroupPrivate> d;
    friend class CGroupDataModel;
    friend class CGroupDataModelPrivate;
};

}
