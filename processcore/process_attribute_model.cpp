/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "process_attribute_model.h"

#include "extended_process_list.h"
#include "process_attribute.h"

using namespace KSysGuard;

class Q_DECL_HIDDEN ProcessAttributeModel::Private
{
public:
    QVector<ProcessAttribute *> m_attributes;
};

ProcessAttributeModel::ProcessAttributeModel(const QVector<ProcessAttribute *> &attributes, QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private)
{
    d->m_attributes = attributes;
}

ProcessAttributeModel::~ProcessAttributeModel()
{
}

int ProcessAttributeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0; // flat list
    }
    return d->m_attributes.count();
}

QVariant ProcessAttributeModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return QVariant();
    }

    auto attribute = d->m_attributes[index.row()];
    switch (static_cast<Role>(role)) {
    case Role::Name:
        return attribute->name();
    case Role::ShortName:
        if (attribute->shortName().isEmpty()) {
            return attribute->name();
        }
        return attribute->shortName();
    case Role::Id:
        return attribute->id();
    case Role::Description:
        return attribute->description();
    case Role::Unit:
        return attribute->unit();
    case Role::Minimum:
        return attribute->min();
    case Role::Maximum:
        return attribute->max();
    }
    return QVariant();
}

QHash<int, QByteArray> ProcessAttributeModel::roleNames() const
{
    return QAbstractListModel::roleNames().unite({
        {static_cast<int>(Role::Id), "id"},
        {static_cast<int>(Role::Name), "name"},
        {static_cast<int>(Role::ShortName), "shortName"},
        {static_cast<int>(Role::Description), "description"},
        {static_cast<int>(Role::Unit), "unit"},
        {static_cast<int>(Role::Minimum), "minimum"},
        {static_cast<int>(Role::Maximum), "maximum"},
    });
}
