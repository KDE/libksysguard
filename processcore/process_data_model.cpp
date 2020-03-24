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

#include "process_data_model.h"
#include "formatter.h"

#include "processcore/extended_process_list.h"
#include "processcore/process.h"
#include "processcore/process_attribute.h"
#include "processcore/process_attribute_model.h"
#include "processcore/process_data_provider.h"

#include <QMetaEnum>
#include <QTimer>

using namespace KSysGuard;

class Q_DECL_HIDDEN KSysGuard::ProcessDataModel::Private
{
public:
    Private(ProcessDataModel *q);
    void beginInsertRow(KSysGuard::Process *parent);
    void endInsertRow();
    void beginRemoveRow(KSysGuard::Process *process);
    void endRemoveRow();

    void update();
    QModelIndex getQModelIndex(Process *process, int column) const;

    ProcessDataModel *q;
    KSysGuard::ExtendedProcesses *m_processes;
    QTimer *m_timer;
    ProcessAttributeModel *m_attributeModel = nullptr;
    const int m_updateInterval = 2000;

    QHash<QString, KSysGuard::ProcessAttribute *> m_availableAttributes;
    QVector<KSysGuard::ProcessAttribute *> m_enabledAttributes;

};

ProcessDataModel::ProcessDataModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new ProcessDataModel::Private(this))
{
}

ProcessDataModel::~ProcessDataModel()
{
}

ProcessDataModel::Private::Private(ProcessDataModel *_q)
    : q(_q)
    , m_processes(new KSysGuard::ExtendedProcesses(_q))
    , m_timer(new QTimer(_q))
{
    connect(m_processes, &KSysGuard::Processes::beginAddProcess, q, [this](KSysGuard::Process *process) {
        beginInsertRow(process);
    });
    connect(m_processes, &KSysGuard::Processes::endAddProcess, q, [this]() {
        endRemoveRow();
    });
    connect(m_processes, &KSysGuard::Processes::beginRemoveProcess, q, [this](KSysGuard::Process *process) {
        beginRemoveRow(process);
    });
    connect(m_processes, &KSysGuard::Processes::endRemoveProcess, q, [this]() {
        endRemoveRow();
    });

    const auto attributes = m_processes->attributes();
    m_availableAttributes.reserve(attributes.count());
    for (auto attr : attributes) {
        m_availableAttributes[attr->id()] = attr;
    }

    connect(m_timer, &QTimer::timeout, q, [this]() {
        update();
    });
    m_timer->setInterval(m_updateInterval);
    m_timer->start();
}

QVariant ProcessDataModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
        return QVariant();
    }

    const int attr = index.column();
    auto attribute = d->m_enabledAttributes[attr];
    switch (role) {
    case Qt::DisplayRole:
    case FormattedValue: {
        KSysGuard::Process *process = reinterpret_cast<KSysGuard::Process *>(index.internalPointer());
        const QVariant value = attribute->data(process);
        return KSysGuard::Formatter::formatValue(value, attribute->unit());
    }
    case Value: {
        KSysGuard::Process *process = reinterpret_cast<KSysGuard::Process *>(index.internalPointer());
        const QVariant value = attribute->data(process);
        return value;
    }
    case Attribute: {
        return attribute->id();
    }
    case Minimum: {
        return attribute->min();
    }
    case Maximum: {
        return attribute->max();
    }
    case ShortName: {
        if (!attribute->shortName().isEmpty()) {
            return attribute->shortName();
        }
        return attribute->name();
    }
    case Name: {
        return attribute->name();
    }
    case Unit: {
        return attribute->unit();
    }
    }
    return QVariant();
}

int ProcessDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0; //In flat mode, none of the processes have children
    return d->m_processes->processCount();
}

QModelIndex ProcessDataModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

QStringList ProcessDataModel::availableAttributes() const
{
    return d->m_availableAttributes.keys();
}

QStringList ProcessDataModel::enabledAttributes() const
{
    QStringList rc;
    rc.reserve(d->m_enabledAttributes.size());
    for (auto attr : d->m_enabledAttributes) {
        rc << attr->id();
    }
    return rc;
}

void ProcessDataModel::setEnabledAttributes(const QStringList &enabledAttributes)
{
    beginResetModel();

    QVector<ProcessAttribute *> unusedAttributes = d->m_enabledAttributes;
    d->m_enabledAttributes.clear();

    for (auto attributeId : enabledAttributes) {
        auto attribute = d->m_availableAttributes[attributeId];
        if (!attribute) {
            continue;
        }
        unusedAttributes.removeOne(attribute);
        d->m_enabledAttributes << attribute;
        int columnIndex = d->m_enabledAttributes.count() - 1;

        // reconnect as using the columnIndex in the lambda makes everything super fast
        disconnect(attribute, &KSysGuard::ProcessAttribute::dataChanged, this, nullptr);
        connect(attribute, &KSysGuard::ProcessAttribute::dataChanged, this, [this, columnIndex](KSysGuard::Process *process) {
            const QModelIndex index = d->getQModelIndex(process, columnIndex);
            emit dataChanged(index, index);
        });

        attribute->setEnabled(true);
    }

    for (auto *unusedAttribute : qAsConst(unusedAttributes)) {
        disconnect(unusedAttribute, &KSysGuard::ProcessAttribute::dataChanged, this, nullptr);
        unusedAttribute->setEnabled(false);
    }

    d->update();
    endResetModel();

    emit enabledAttributesChanged();
}

QModelIndex ProcessDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0) {
        return QModelIndex();
    }
    if (column < 0 || column >= columnCount()) {
        return QModelIndex();
    }

    if (parent.isValid()) {
        return QModelIndex();
    }
    if (row >= d->m_processes->processCount()) {
        return QModelIndex();
    }
    return createIndex(row, column, d->m_processes->getAllProcesses().at(row));
}

void ProcessDataModel::Private::beginInsertRow(KSysGuard::Process *process)
{
    Q_ASSERT(process);
    const int row = m_processes->processCount();
    q->beginInsertRows(QModelIndex(), row, row);
}

void ProcessDataModel::Private::endInsertRow()
{
    q->endInsertRows();
}

void ProcessDataModel::Private::beginRemoveRow(KSysGuard::Process *process)
{
    q->beginRemoveRows(QModelIndex(), process->index(), process->index());
}

void ProcessDataModel::Private::endRemoveRow()
{
    q->endRemoveRows();
}

void ProcessDataModel::Private::update()
{
    m_processes->updateAllProcesses(m_updateInterval, KSysGuard::Processes::StandardInformation | KSysGuard::Processes::IOStatistics);
}

QModelIndex ProcessDataModel::Private::getQModelIndex(KSysGuard::Process *process, int column) const
{
    Q_ASSERT(process);
    if (process->pid() == -1)
        return QModelIndex(); // pid -1 is our fake process meaning the very root (never drawn).  To represent that, we return QModelIndex() which also means the top element
    const int row = process->index();
    Q_ASSERT(row != -1);
    return q->createIndex(row, column, process);
}

ProcessAttributeModel *ProcessDataModel::attributesModel()
{
    // lazy load
    if (!d->m_attributeModel) {
        d->m_attributeModel = new KSysGuard::ProcessAttributeModel(d->m_processes, this);
    }
    return d->m_attributeModel;
}

int ProcessDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->m_enabledAttributes.count();
}

QHash<int, QByteArray> ProcessDataModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

    const QMetaEnum e = QMetaEnum::fromType<AdditionalRoles>();

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }

    return roles;
}

QVariant ProcessDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) {
        return QVariant();
    }

    if (section < 0 || section >= columnCount()) {
        return QVariant();
    }

    auto attribute = d->m_enabledAttributes[section];

    switch (role) {
    case Qt::DisplayRole:
    case ShortName: {
        if (!attribute->shortName().isEmpty()) {
            return attribute->shortName();
        }
        return attribute->name();
    }
    case Name:
        return attribute->name();
    case Value:
    case Attribute: {
        return attribute->id();
    }
    case Unit: {
        auto attribute = d->m_enabledAttributes[section];
        return attribute->unit();
    }
    case Minimum: {
        return attribute->min();
    }
    case Maximum: {
        return attribute->max();
    }
    default:
        break;
    }

    return QVariant();
}

#include "process_data_model.moc"
