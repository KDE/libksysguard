/*
    Copyright (c) 2019 Eike Hein <hein@kde.org>
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
#include <QAbstractItemModel>
#include "sensors_export.h"

namespace KSysGuard
{

class SensorInfo;

/**
 * A model representing a tree of sensors that are available from the daemon.
 *
 * This model exposes the daemon's sensors as a tree, based on their path. Each
 * sensor is assumed to be structured in a format similar to
 * `category/object/sensor`. This model will then expose a tree, with `category`
 * as top level, `object` below it and finally `sensor` itself.
 */
class SENSORS_EXPORT SensorTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum AdditionalRoles {
        SensorId = Qt::UserRole + 1,
    };
    Q_ENUM(AdditionalRoles)

    explicit SensorTreeModel(QObject *parent = nullptr);
    virtual ~SensorTreeModel();

    QHash<int, QByteArray> roleNames() const override;
    QVariant headerData(int section, Qt::Orientation, int role) const override;
    QStringList mimeTypes() const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;


private:
    void init();
    void onSensorAdded(const QString &sensor);
    void onSensorRemoved(const QString &sensor);
    void onMetaDataChanged(const QString &sensorId, const SensorInfo &info);

    class Private;
    const std::unique_ptr<Private> d;
};

}