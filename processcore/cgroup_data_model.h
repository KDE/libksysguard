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

#include <QAbstractItemModel>

#include "process_attribute_model.h"

namespace KSysGuard {

class CGroup;

class CGroupDataModelPrivate;

/**
 * @brief The CGroupDataModel class is a list model of all cgroups from a given root
 * Data is exposed as per ProcessDataModel with configurable columns
 *
 * Data is refreshed on a timer
 */
class Q_DECL_EXPORT CGroupDataModel : public QAbstractItemModel
{
    Q_OBJECT
    /**
     * @copydoc ProcessDataModel::availableAttributes
     */
    Q_PROPERTY(QStringList availableAttributes READ availableAttributes CONSTANT)
    /**
     * @copydoc ProcessDataModel::enabledAttributes
     */
    Q_PROPERTY(QStringList enabledAttributes READ enabledAttributes WRITE setEnabledAttributes NOTIFY enabledAttributesChanged)
    /**
     * @copydoc ProcessDataModel::attributesModel
     */
    Q_PROPERTY(QObject* attributesModel READ attributesModel CONSTANT)
    /**
     * @copydoc ProcessDataModel::enabled
     */
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    /**
     * @copydoc setRoot
     */
    Q_PROPERTY(QString setRoot READ root WRITE setRoot NOTIFY rootChanged)

public:
    CGroupDataModel(QObject *parent = nullptr);
    ~CGroupDataModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @copydoc ProcessDataModel::availableAttributes
     */
    QStringList availableAttributes() const;
    /**
     * @copydoc ProcessDataModel::enabledAttributes
     */
    QStringList enabledAttributes() const;
    /**
     * @copydoc ProcessDataModel::setEnabledAttributes
     */
    void setEnabledAttributes(const QStringList &enabledAttributes);

    QModelIndex getQModelIndex(CGroup *cgroup, int column) const;

    /**
     * @copydoc ProcessDataModel::attributesModel
     */
    ProcessAttributeModel *attributesModel();
    /**
     * @copydoc ProcessDataModel::isEnabled
     */
    bool isEnabled() const;
    /**
     * @copydoc ProcessDataModel::setEnabled
     */
    void setEnabled(bool isEnabled);

    QString root() const;
    /**
     * Set the root cgroup to start listing from
     * e.g "user.slice/user-1000.slice"
     *
     * The default is blank
     */
    void setRoot(const QString &root);

    /**
     * Trigger an update of the model
     */
    void update();

Q_SIGNALS:
    void enabledAttributesChanged();
    void enabledChanged();
    void rootChanged();

protected:
    virtual bool filterAcceptsCGroup(const QString &id);

private:
    QScopedPointer<CGroupDataModelPrivate> d;
    void update(CGroup *node);
};

}
