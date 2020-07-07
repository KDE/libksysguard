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

#include "SensorTreeModel.h"

#include <KLocalizedString>
#include <QDebug>
#include <QMetaEnum>
#include <QMimeData>
#include <QRegularExpression>

#include "formatter/Formatter.h"

#include "Sensor.h"
#include "SensorDaemonInterface_p.h"
#include "SensorInfo_p.h"
#include "SensorQuery.h"
#include "SensorGroup_p.h"

using namespace KSysGuard;

struct Q_DECL_HIDDEN SensorTreeItem
{
    SensorTreeItem *parent = nullptr;
    QString segment;
    QMap<QString, SensorTreeItem *> children;

    inline int indexOf(const QString &segment)
    {
        int index = 0;
        for (auto child : qAsConst(children)) {
            if (child->segment == segment) {
                return index;
            }
            index++;
        }

        return -1;
    }

    inline SensorTreeItem *itemAt(int index) {
        int currentIndex = 0;
        for (auto child : qAsConst(children)) {
            if (currentIndex++ == index) {
                return child;
            }
        }
        return nullptr;
    }

    ~SensorTreeItem()
    {
        qDeleteAll(children);
    }
};

class Q_DECL_HIDDEN SensorTreeModel::Private
{
public:
    Private(SensorTreeModel *qq)
        : rootItem(new SensorTreeItem)
        , q(qq)
    {
    }
    ~Private()
    {
        delete rootItem;
    }

    SensorTreeItem *rootItem;
    QHash<SensorTreeItem *, SensorInfo> sensorInfos;

    void addSensor(const QString &sensorId, const SensorInfo &info);
    void removeSensor(const QString &sensorId);

    QString sensorId(const QModelIndex &index);

    SensorTreeItem *find(const QString &sensorId);

    QHash<QString, int> m_groupMatches;

private:
    SensorTreeModel *q;
};

void SensorTreeModel::Private::addSensor(const QString &sensorId, const SensorInfo &info)
{
    const QStringList &segments = sensorId.split(QLatin1Char('/'));

    if (!segments.count() || segments.at(0).isEmpty()) {
        qDebug() << "Rejecting sensor" << sensorId << "- sensor id is not well-formed.";
        return;
    }

    QString sensorIdExpr = groupRegexForId(sensorId);

    if (!sensorIdExpr.isEmpty()) {
        if (m_groupMatches.contains(sensorIdExpr)) {
            m_groupMatches[sensorIdExpr]++;
        } else {
            m_groupMatches[sensorIdExpr] = 1;
        }
        
        if (m_groupMatches[sensorIdExpr] == 2) {
            SensorInfo newInfo;
            newInfo.name = sensorNameForRegEx(sensorIdExpr);
            newInfo.description = info.description;
            newInfo.variantType = info.variantType;
            KSysGuard::Unit unit = info.unit;
            newInfo.min = info.min;
            newInfo.max = info.max;
            
            addSensor(sensorIdExpr, newInfo);
        }
    }
    SensorTreeItem *item = rootItem;

    for (auto segment : segments) {
        auto child = item->children.value(segment, nullptr);

        if (child) {
            item = child;

        } else {
            SensorTreeItem *newItem = new SensorTreeItem();
            newItem->parent = item;
            newItem->segment = segment;

            const QModelIndex &parentIndex = (item == rootItem) ? QModelIndex() : q->createIndex(item->parent->indexOf(item->segment), 0, item);

            auto index = std::distance(item->children.begin(), item->children.upperBound(segment));

            q->beginInsertRows(parentIndex, index, index);
            item->children.insert(segment, newItem);
            q->endInsertRows();

            item = newItem;
        }
    }

    sensorInfos[item] = info;
}

void SensorTreeModel::Private::removeSensor(const QString &sensorId)
{
    if (sensorId.contains(QRegularExpression(QStringLiteral("\\d+")))) {
        QString sensorIdExpr = sensorId;
        sensorIdExpr.replace(QRegularExpression(QStringLiteral("(\\d+)")), QStringLiteral("\\d+"));
        
        if (m_groupMatches[sensorIdExpr] == 1) {
            m_groupMatches.remove(sensorIdExpr);
            removeSensor(sensorIdExpr);
        } else if (m_groupMatches.contains(sensorIdExpr)) {
            m_groupMatches[sensorIdExpr]--;
        }
    }

    SensorTreeItem *item = find(sensorId);
    if (!item) {
        return;
    }

    SensorTreeItem *parent = item->parent;
    if (!parent) {
        return;
    }

    auto remove = [this](SensorTreeItem *item, SensorTreeItem *parent) {
        const int index = item->parent->indexOf(item->segment);

        const QModelIndex &parentIndex = (parent == rootItem) ? QModelIndex() : q->createIndex(parent->parent->indexOf(parent->segment), 0, parent);
        q->beginRemoveRows(parentIndex, index, index);
        delete item->parent->children.take(item->segment);
        q->endRemoveRows();

        sensorInfos.remove(item);
    };

    remove(item, parent);

    while (!parent->children.count()) {
        item = parent;
        parent = parent->parent;

        if (!parent) {
            break;
        }

        remove(item, parent);
    }
}

QString SensorTreeModel::Private::sensorId(const QModelIndex &index)
{
    QStringList segments;

    SensorTreeItem *item = static_cast<SensorTreeItem *>(index.internalPointer());

    segments << item->segment;

    while (item->parent && item->parent != rootItem) {
        item = item->parent;
        segments.prepend(item->segment);
    }

    return segments.join(QLatin1Char('/'));
}

SensorTreeItem *KSysGuard::SensorTreeModel::Private::find(const QString &sensorId)
{
    auto item = rootItem;
    const auto segments = sensorId.split(QLatin1Char('/'));
    for (const QString &segment : segments) {
        item = item->children.value(segment, nullptr);
        if (!item) {
            return nullptr;
        }
    }
    return item;
}

SensorTreeModel::SensorTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new Private(this))
{
    connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::sensorAdded, this, &SensorTreeModel::onSensorAdded);
    connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::sensorRemoved, this, &SensorTreeModel::onSensorRemoved);
    connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::metaDataChanged, this, &SensorTreeModel::onMetaDataChanged);
    init();
}

SensorTreeModel::~SensorTreeModel()
{
}

QHash<int, QByteArray> SensorTreeModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("AdditionalRoles"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }

    return roles;
}

QVariant SensorTreeModel::headerData(int section, Qt::Orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (section == 0) {
        return i18n("Sensor Browser");
    }

    return QVariant();
}

QStringList SensorTreeModel::mimeTypes() const
{
    return QStringList() << QStringLiteral("application/x-ksysguard");
}

QVariant SensorTreeModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        SensorTreeItem *item = static_cast<SensorTreeItem *>(index.internalPointer());

        if (d->sensorInfos.contains(item)) {
            auto info = d->sensorInfos.value(item);
            const QString &unit = Formatter::symbol(info.unit);

            if (!unit.isEmpty()) {
                return i18nc("Name (unit)", "%1 (%2)", info.name, unit);
            }

            return info.name;
        }

        return segmentNameForRegEx(item->segment);
    // Only leaf nodes are valid sensors
    } else if (role == SensorId) {
        if (rowCount(index)) {
            return QString();
        } else {
            return d->sensorId(index);
        }
    }

    return QVariant();
}

QMimeData *SensorTreeModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();

    if (indexes.count() != 1) {
        return mimeData;
    }

    const QModelIndex &index = indexes.at(0);

    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return mimeData;
    }

    if (rowCount(index)) {
        return mimeData;
    }

    mimeData->setData(QStringLiteral("application/x-ksysguard"), d->sensorId(index).toUtf8());

    return mimeData;
}

Qt::ItemFlags SensorTreeModel::flags(const QModelIndex &index) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return Qt::NoItemFlags;
    }

    if (!rowCount(index)) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    return Qt::ItemIsEnabled;
}

int SensorTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        if (!checkIndex(parent, CheckIndexOption::IndexIsValid)) {
            return 0;
        }

        const SensorTreeItem *item = static_cast<SensorTreeItem *>(parent.internalPointer());
        return item->children.count();
    }

    return d->rootItem->children.count();
}

int SensorTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 1;
}

QModelIndex SensorTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    SensorTreeItem *parentItem = d->rootItem;

    if (parent.isValid()) {
        if (parent.model() != this) {
            return QModelIndex();
        }

        parentItem = static_cast<SensorTreeItem *>(parent.internalPointer());
    }

    if (row < 0 || row >= parentItem->children.count()) {
        return QModelIndex();
    }

    if (column < 0) {
        return QModelIndex();
    }

    return createIndex(row, column, parentItem->itemAt(row));
}

QModelIndex SensorTreeModel::parent(const QModelIndex &index) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::DoNotUseParent)) {
        return QModelIndex();
    }

    if (index.column() > 0) {
        return QModelIndex();
    }

    const SensorTreeItem *item = static_cast<SensorTreeItem *>(index.internalPointer());
    SensorTreeItem *parentItem = item->parent;

    if (parentItem == d->rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->parent->indexOf(parentItem->segment), 0, parentItem);
}

void SensorTreeModel::init()
{
    auto query = new SensorQuery{QString(), this};
    connect(query, &SensorQuery::finished, [query, this]() {
        query->deleteLater();
        const auto result = query->result();
        beginResetModel();
        for (auto pair : result) {
            d->addSensor(pair.first, pair.second);
        }
        endResetModel();
    });
    query->execute();
}

void KSysGuard::SensorTreeModel::onSensorAdded(const QString &sensor)
{
    SensorDaemonInterface::instance()->requestMetaData(sensor);
}

void KSysGuard::SensorTreeModel::onSensorRemoved(const QString &sensor)
{
    d->removeSensor(sensor);
}

void KSysGuard::SensorTreeModel::onMetaDataChanged(const QString &sensorId, const SensorInfo &info)
{
    auto item = d->find(sensorId);
    if (!item) {
        d->addSensor(sensorId, info);
    } else {
        d->sensorInfos[item] = info;

        auto parentItem = item->parent;
        if (!parentItem) {
            return;
        }

        auto parentIndex = QModelIndex{};
        if (parentItem != d->rootItem) {
            parentIndex = createIndex(parentItem->parent->indexOf(parentItem->segment), 0, parentItem);
        }

        auto itemIndex = index(parentItem->indexOf(item->segment), 0, parentIndex);
        Q_EMIT dataChanged(itemIndex, itemIndex);
    }
}
