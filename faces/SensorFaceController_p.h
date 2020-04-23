/*
    Copyright (C) 2020 Marco Martin <mart@kde.org>

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

#include <QQuickItem>
#include <QStandardItemModel>

class SensorFaceController;
class QQmlEngine;

class FacesModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum AdditionalRoles {
        PluginIdRole = Qt::UserRole + 1,
    };
    Q_ENUM(AdditionalRoles)

    FacesModel(QObject *parent = nullptr);
    ~FacesModel() = default;

    Q_INVOKABLE void reload();
    Q_INVOKABLE QString pluginId(int row);

    QHash<int, QByteArray> roleNames() const override;
};

class PresetsModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum AdditionalRoles {
        PluginIdRole = Qt::UserRole + 1,
        ConfigRole,
        WritableRole
    };
    Q_ENUM(AdditionalRoles)

    PresetsModel(QObject *parent = nullptr);
    ~PresetsModel() = default;

    Q_INVOKABLE void reload();

    QHash<int, QByteArray> roleNames() const override;
};

