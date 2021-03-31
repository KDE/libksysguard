/*
    Copyright (C) 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

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

#include "org.kde.ksystemstats.h"
#include "systemstats_export.h"

namespace KSysGuard
{
namespace SystemStats
{

const QString ServiceName = QStringLiteral("org.kde.ksystemstats");
const QString ObjectPath = QStringLiteral("/");

/**
 * This exposes the generated DBus interface for org.kde.ksystemstats
 */
class SYSTEMSTATS_EXPORT DBusInterface : public org::kde::ksystemstats
{
    Q_OBJECT
public:
    DBusInterface(const QString &service = ServiceName, const QString &path = ObjectPath, const QDBusConnection &connection = QDBusConnection::sessionBus(), QObject *parent = nullptr);
};

} // namespace SystemStats
} // namespace KSysGuard
