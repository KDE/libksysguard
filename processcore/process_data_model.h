/*
    Copyright (c) 2020 David Edmundson <davidedmundson@kde.org>

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

#include <QAbstractItemModel>
#include <processcore/processes.h>

namespace KSysGuard
{

class Process;
class ProcessAttributeModel;

/**
 * This class contains a model of all running processes
 * Rows represent processes
 * Columns represent a specific attribute, such as CPU usage
 * Attributes can be enabled or disabled
 *
 * This class abstracts the process data so that it can be presented without the client
 * needing to understand the semantics of each column
 * It is designed to be consumable by a QML API
 */
class Q_DECL_EXPORT ProcessDataModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList availableAttributes READ availableAttributes CONSTANT)
    Q_PROPERTY(QStringList enabledAttributes READ enabledAttributes WRITE setEnabledAttributes NOTIFY enabledAttributesChanged)
    Q_PROPERTY(QAbstractItemModel *attributesModel READ attributesModel CONSTANT)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    enum AdditionalRoles {
        Value = Qt::UserRole, /// The raw value of the attribute. This is unformatted and could represent an int, real or string
        FormattedValue, /// A string containing the value in a locale friendly way with appropriate suffix "eg. 5Mb" "20%"

        PIDs, /// The PIDs associated with this row
        Minimum, /// Smallest value this reading can be in normal situations. A hint for graphing utilities
        Maximum, /// Largest value this reading can be in normal situations. A hint for graphing utilities

        Attribute, /// The attribute id associated with this column
        Name, /// The full name of this attribute
        ShortName, /// A shorter name of this attribute, compressed for viewing
        Unit, /// The unit associated with this attribute. Returned value is of the type KSysGuard::Unit
    };
    Q_ENUM(AdditionalRoles)

    explicit ProcessDataModel(QObject *parent = nullptr);
    ~ProcessDataModel() override;

    /**
     * A list of attribute IDs that can be enabled
     */
    QStringList availableAttributes() const;

    /**
     * The list of available attributes that can be enabled, presented as a model
     * See @availableAttributes
     */
    ProcessAttributeModel *attributesModel();

    /**
     * The currently enabled attributes
     */
    QStringList enabledAttributes() const;
    /**
     * Select which process attributes should be enabled
     * The order determines the column order
     *
     * The default value is empty
     */
    void setEnabledAttributes(const QStringList &enabledAttributes);

    bool enabled() const;
    void setEnabled(bool newEnabled);
    Q_SIGNAL void enabledChanged();

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

Q_SIGNALS:
    void enabledAttributesChanged();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
