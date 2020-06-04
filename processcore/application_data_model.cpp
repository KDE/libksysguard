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

#include "application_data_model.h"
#include <KUser>

#include <QDebug>

using namespace KSysGuard;

ApplicationDataModel::ApplicationDataModel(QObject *parent)
    : CGroupDataModel(parent)
{
    const QString userId = KUserId::currentEffectiveUserId().toString();
    setRoot(QStringLiteral("/user.slice/user-%1.slice/user@%1.service").arg(userId));
}

bool ApplicationDataModel::filterAcceptsCGroup(const QString &id)
{
    if (!CGroupDataModel::filterAcceptsCGroup(id)) {
        return false;
    }
    // this class is all temporary. In the future as per https://systemd.io/DESKTOP_ENVIRONMENTS/
    // all apps will have a managed by a drop-in that puts apps in the app.slice
    // when this happens adjust the root above and drop this filterAcceptsCGroup line
   return id.contains(QLatin1String("/app")) || (id.contains(QLatin1String("/flatpak")) && id.endsWith(QLatin1String("scope")));
}
