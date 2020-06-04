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

#include "cgroup_data_model.h"

#include "cgroup.h"
#include "extended_process_list.h"
#include "process_attribute.h"
#include "process_data_model.h"
#include "Formatter.h"

#include <KLocalizedString>

#include <QMetaEnum>
#include <QTimer>
#include <QDebug>
#include <QDir>

#include <algorithm>

using namespace  KSysGuard;

class KSysGuard::CGroupDataModelPrivate
{
public:
    ExtendedProcesses *m_processes;
    QTimer *m_updateTimer;
    ProcessAttributeModel *m_attributeModel = nullptr;
    QHash<QString, KSysGuard::ProcessAttribute* > m_availableAttributes;
    QVector<KSysGuard::ProcessAttribute* > m_enabledAttributes;

    QString m_root;
    QScopedPointer<CGroup> m_rootGroup;

    QVector<CGroup *> m_cGroups; // an ordered list of unfiltered cgroups from our root
    QHash<QString, CGroup *> m_cgroupMap; // all known cgroups from our root
    QHash<QString, CGroup *> m_oldGroups;
};

class GroupNameAttribute : public ProcessAttribute
{
public:
    GroupNameAttribute(QObject *parent) :
        KSysGuard::ProcessAttribute(QStringLiteral("menuId"), i18nc("@title", "Desktop ID"), parent) {
    }
    QVariant cgroupData(CGroup *app) const override {
        return app->service()->menuId();
    }
};

class AppIconAttribute : public KSysGuard::ProcessAttribute
{
public:
    AppIconAttribute(QObject *parent) :
        KSysGuard::ProcessAttribute(QStringLiteral("iconName"), i18nc("@title", "Icon"), parent) {
    }
    QVariant cgroupData(CGroup *app) const override {
        return app->service()->icon();
    }
};

class AppNameAttribute : public KSysGuard::ProcessAttribute
{
public:
    AppNameAttribute(QObject *parent) :
        KSysGuard::ProcessAttribute(QStringLiteral("appName"), i18nc("@title", "Name"), parent) {
    }
    QVariant cgroupData(CGroup *app) const override {
        return app->service()->name();
    }
};

CGroupDataModel::CGroupDataModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new CGroupDataModelPrivate)
{
    d->m_updateTimer = new QTimer(this);
    d->m_processes = new ExtendedProcesses(this);

    QVector<ProcessAttribute *> attributes = d->m_processes->attributes();
    attributes.reserve(attributes.count() + 3);
    attributes.append(new GroupNameAttribute(this));
    attributes.append(new AppNameAttribute(this));
    attributes.append(new AppIconAttribute(this));
    for (auto attr : qAsConst(attributes)) {
        d->m_availableAttributes[attr->id()] = attr;
    }

    connect(d->m_updateTimer, &QTimer::timeout, this, [this]() {
        update();
    });
    d->m_updateTimer->setInterval(2000);
    d->m_updateTimer->start();

    setRoot(QStringLiteral("/"));
}

CGroupDataModel::~CGroupDataModel()
{
}

int CGroupDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->m_cGroups.count();
}

QModelIndex CGroupDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || row >= d->m_cGroups.count()) {
        return QModelIndex();
    }
    if (parent.isValid()) {
        return QModelIndex();
    }
    return createIndex(row, column, d->m_cGroups.at(row));
}

QModelIndex CGroupDataModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int CGroupDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->m_enabledAttributes.count();
}

QStringList CGroupDataModel::availableAttributes() const
{
    return d->m_availableAttributes.keys();
}

QStringList CGroupDataModel::enabledAttributes() const
{
    QStringList rc;
    rc.reserve(d->m_enabledAttributes.size());
    for (auto attr : qAsConst(d->m_enabledAttributes)) {
        rc << attr->id();
    }
    return rc;
}

void CGroupDataModel::setEnabledAttributes(const QStringList &enabledAttributes)
{
     beginResetModel();

     QVector<ProcessAttribute*> unusedAttributes = d->m_enabledAttributes;
     d->m_enabledAttributes.clear();

     for (auto attribute: enabledAttributes) {
         auto attr = d->m_availableAttributes.value(attribute, nullptr);
         if (!attr) {
             qWarning() << "Could not find attribute" << attribute;
             continue;
         }
         unusedAttributes.removeOne(attr);
         d->m_enabledAttributes << attr;
         int columnIndex = d->m_enabledAttributes.count() - 1;

         // reconnect as using the attribute in the lambda makes everything super fast
         disconnect(attr, &KSysGuard::ProcessAttribute::dataChanged, this, nullptr);
         connect(attr, &KSysGuard::ProcessAttribute::dataChanged, this, [this, columnIndex](KSysGuard::Process *process) {
             auto cgroup = d->m_cgroupMap.value(process->cGroup());
             if (!cgroup) {
                 return;
             }
             const QModelIndex index = getQModelIndex(cgroup, columnIndex);
             emit dataChanged(index, index);
         });

         attr->setEnabled(true);
     }

     for (auto unusedAttr : qAsConst(unusedAttributes)) {
         disconnect(unusedAttr, &KSysGuard::ProcessAttribute::dataChanged, this, nullptr);
         unusedAttr->setEnabled(false);
     }

     endResetModel();

     emit enabledAttributesChanged();
}

QModelIndex CGroupDataModel::getQModelIndex(CGroup *cgroup, int column) const
{
    Q_ASSERT(cgroup);
    int row = d->m_cGroups.indexOf(cgroup);
    return index(row, column, QModelIndex());
}

QHash<int, QByteArray> CGroupDataModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    QMetaEnum e = ProcessDataModel::staticMetaObject.enumerator(ProcessDataModel::staticMetaObject.indexOfEnumerator("AdditionalRoles"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }

    return roles;
}

QVariant CGroupDataModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return QVariant();
    }
    int attr = index.column();
    auto attribute = d->m_enabledAttributes[attr];
    switch(role) {
        case Qt::DisplayRole:
        case ProcessDataModel::FormattedValue: {
            KSysGuard::CGroup *app = reinterpret_cast< KSysGuard::CGroup* > (index.internalPointer());
            const QVariant value = attribute->cgroupData(app);
            return KSysGuard::Formatter::formatValue(value, attribute->unit());
        }
        case ProcessDataModel::Value: {
            KSysGuard::CGroup *app = reinterpret_cast< KSysGuard::CGroup* > (index.internalPointer());
            const QVariant value = attribute->cgroupData(app);
            return value;
        }
        case ProcessDataModel::Attribute: {
            return attribute->id();
        }
        case ProcessDataModel::Minimum: {
            return attribute->min();
        }
        case ProcessDataModel::Maximum: {
            return attribute->max();
        }
        case ProcessDataModel::ShortName: {
            if (!attribute->shortName().isEmpty()) {
                return attribute->shortName();
            }
            return attribute->name();
        }
        case ProcessDataModel::Name: {
            return attribute->name();
        }
        case ProcessDataModel::Unit: {
            return attribute->unit();
        }
        case ProcessDataModel::PIDs: {
            KSysGuard::CGroup *app = reinterpret_cast< KSysGuard::CGroup* > (index.internalPointer());
            QVariantList pidList;
            std::transform(app->processes().constBegin(), app->processes().constEnd(), std::back_inserter(pidList), [](Process* process) -> QVariant {
                return QVariant::fromValue(process->pid());
            });
            return pidList;
        }
    }
    return QVariant();
}

QVariant CGroupDataModel::headerData(int section, Qt::Orientation orientation, int role) const
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
    case ProcessDataModel::ShortName: {
        if (!attribute->shortName().isEmpty()) {
            return attribute->shortName();
        }
        return attribute->name();
    }
    case ProcessDataModel::Name:
        return attribute->name();
    case ProcessDataModel::Value:
    case ProcessDataModel::Attribute: {
        return attribute->id();
    }
    case ProcessDataModel::Unit: {
        auto attribute = d->m_enabledAttributes[section];
        return attribute->unit();
    }
    case ProcessDataModel::Minimum: {
        return attribute->min();
    }
    case ProcessDataModel::Maximum: {
        return attribute->max();
    }
    default:
        break;
    }

    return QVariant();
}

ProcessAttributeModel *CGroupDataModel::attributesModel()
{
    //lazy load
    if (!d->m_attributeModel) {
        d->m_attributeModel = new KSysGuard::ProcessAttributeModel(d->m_processes, this);
    }
    return d->m_attributeModel;
}

bool CGroupDataModel::isEnabled() const
{
    return d->m_updateTimer->isActive();
}

void CGroupDataModel::setEnabled(bool enabled)
{
    if (enabled) {
        d->m_updateTimer->start();
    } else {
        d->m_updateTimer->stop();
    }
}

QString CGroupDataModel::root() const
{
    return d->m_root;
}

void CGroupDataModel::setRoot(const QString &root)
{
    if (root == d->m_root) {
        return;
    }
    d->m_root = root;
    d->m_rootGroup.reset(new CGroup(root));
    emit rootChanged();
    update();
}

void CGroupDataModel::update()
{
    if (!d->m_rootGroup) {
        return;
    }

    d->m_oldGroups = d->m_cgroupMap;

    // In an ideal world we would only the relevant process
    // but Ksysguard::Processes doesn't handle that very well
    d->m_processes->updateAllProcesses();

    update(d->m_rootGroup.data());

    for (auto c : qAsConst(d->m_oldGroups)) {
        int row = d->m_cGroups.indexOf(c);
        if (row >= 0) {
            beginRemoveRows(QModelIndex(), row, row);
            d->m_cGroups.removeOne(c);
            endRemoveRows();
        }
        d->m_cgroupMap.remove(c->id());
        delete c;
    }
}

bool CGroupDataModel::filterAcceptsCGroup(const QString &id)
{
    return id.endsWith(QLatin1String(".service")) || id.endsWith(QLatin1String(".scope"));
}

void CGroupDataModel::update(CGroup *node)
{
    const QString path = CGroup::cgroupSysBasePath() + node->id();
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }

    // Update our own stat info
    // This may trigger some dataChanged
    const QVector<pid_t> pids = node->getPids();
    QVector<Process*> processes;
    for (const pid_t pid : pids) {
        auto proc = d->m_processes->getProcess(pid);
        if (proc) { // as potentially this is racey with when kprocess fetched data
            processes << proc;
        }
    }
    node->setProcesses(processes);

    const auto entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto subDir : entries) {
        const QString childId = node->id() % QLatin1Char('/') % subDir;
        CGroup *childNode = d->m_cgroupMap[childId];
        if (!childNode) {
            childNode = new CGroup(childId);
            d->m_cgroupMap[childNode->id()] = childNode;

            if (filterAcceptsCGroup(childId)) {
                int row = d->m_cGroups.count();
                beginInsertRows(QModelIndex(), row, row);
                d->m_cGroups.append(childNode);
                endInsertRows();
            }
        }
        update(childNode);
        d->m_oldGroups.remove(childId);
    }
}
