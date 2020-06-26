/*
    Copyright (c) 2020 David Edmundson <davidedmundson@kde.org>
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

#pragma once

#include <QAbstractListModel>

namespace KSysGuard
{

class ExtendedProcesses;
class ProcessAttribute;

/**
 * Presents a list of available attributes that can be
 * enabled on a ProceessDataModel
 */
class Q_DECL_EXPORT ProcessAttributeModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        Name = Qt::DisplayRole, /// Human readable translated name of the attribute
        Id = Qt::UserRole, /// Computer readable ID of the attribute
        ShortName = Qt::UserRole + 1, /// A shorter human readable translated name of the attribute
        Description, /// A longer, sentence-based description of the attribute
        Unit, /// The unit, of type KSysGuard::Unit
    };
    Q_ENUM(Role);

    ProcessAttributeModel(const QVector<ProcessAttribute *> &attributes, QObject *parent = nullptr);
    ~ProcessAttributeModel() override;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    class Private;
    QScopedPointer<Private> d;
};

}
