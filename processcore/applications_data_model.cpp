#include "applications_data_model.h"

#include "applications.h"
#include "application.h"
#include "extended_process_list.h"
#include "process_attribute.h"
#include "process_data_model.h"
#include "Formatter.h"

#include <KLocalizedString>

#include <QMetaEnum>
#include <QTimer>
#include <QDebug>
#include <algorithm>

using namespace  KSysGuard;

class Q_DECL_HIDDEN ApplicationsDataModel::Private
{
public:
    ExtendedProcesses *m_processes;
    Applications *m_apps;
    QHash<QString, KSysGuard::ProcessAttribute* > m_availableAttributes;
    QVector<KSysGuard::ProcessAttribute* > m_enabledAttributes;
    ProcessAttributeModel *m_attributeModel = nullptr;
    QTimer *m_updateTimer;
};

class GroupNameAttribute : public ProcessAttribute
{
public:
    GroupNameAttribute(QObject *parent) :
        KSysGuard::ProcessAttribute(QStringLiteral("menuId"), i18n("Desktop ID"), parent) {
    }
    QVariant appData(Application *app) const override {
        return app->service()->menuId();
    }
};

class AppIconAttribute : public KSysGuard::ProcessAttribute
{
public:
    AppIconAttribute(QObject *parent) :
        KSysGuard::ProcessAttribute(QStringLiteral("iconName"), i18n("Icon"), parent) {
    }
    QVariant appData(Application *app) const override {
        return app->service()->icon();
    }
};

class AppNameAttribute : public KSysGuard::ProcessAttribute
{
public:
    AppNameAttribute(QObject *parent) :
        KSysGuard::ProcessAttribute(QStringLiteral("appName"), i18n("Name"), parent) {
    }
    QVariant appData(Application *app) const override {
        return app->service()->name();
    }
};

ApplicationsDataModel::ApplicationsDataModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new Private)
{
    d->m_updateTimer = new QTimer(this);
    d->m_processes = new ExtendedProcesses(this);
    d->m_apps = new Applications(d->m_processes, this);
    connect(d->m_apps, &Applications::beginApplicationAdded, this, [this](Application *app) {
        Q_UNUSED(app)
        int count = d->m_apps->getAllApplications().count();
        beginInsertRows(QModelIndex(), count, count);
    });
    connect(d->m_apps, &Applications::endApplicationAdded, this, [this]() {
        endInsertRows();
    });
    connect(d->m_apps, &Applications::beginApplicationRemoved, this, [this](Application *app) {
        int index = d->m_apps->getAllApplications().indexOf(app);
        Q_ASSERT(index >= 0);
        beginRemoveRows(QModelIndex(), index, index);
    });
    connect(d->m_apps, &Applications::endApplicationRemoved, this, [this]() {
        endRemoveRows();
    });

    //connect app changes -> emit changed
    connect(d->m_apps, &Applications::applicationChanged, this, [this](Application *app) {
       emit dataChanged(getQModelIndex(app, 0), getQModelIndex(app, columnCount())); //cgroup adjusted so every column changes
    });

    QVector<ProcessAttribute *> attributes = d->m_processes->attributes();
    attributes.reserve(attributes.count() + 3);
    attributes.append(new GroupNameAttribute(this));
    attributes.append(new AppNameAttribute(this));
    attributes.append(new AppIconAttribute(this));
    for (auto attr: attributes) {
        d->m_availableAttributes[attr->id()] = attr;
    }

    connect(d->m_updateTimer, &QTimer::timeout, this, [this]() {
        d->m_processes->updateAllProcesses(2000, KSysGuard::Processes::StandardInformation | KSysGuard::Processes::IOStatistics);
    });
    d->m_updateTimer->setInterval(2000);
    d->m_updateTimer->start();
    d->m_processes->updateAllProcesses(2000, KSysGuard::Processes::StandardInformation | KSysGuard::Processes::IOStatistics);
}

ApplicationsDataModel::~ApplicationsDataModel()
{
}

int ApplicationsDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->m_apps->getAllApplications().count();
}

QModelIndex ApplicationsDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if(row<0) return QModelIndex();
    if(column<0 || column >= columnCount() ) return QModelIndex();

    if( parent.isValid()) return QModelIndex();
    if( d->m_apps->getAllApplications().count() <= row) return QModelIndex();
    return createIndex(row, column, d->m_apps->getAllApplications().at(row));
}

QModelIndex ApplicationsDataModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int ApplicationsDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->m_enabledAttributes.count();
}

QStringList ApplicationsDataModel::availableAttributes() const
{
    return d->m_availableAttributes.keys();
}

QStringList ApplicationsDataModel::enabledAttributes() const
{
    QStringList rc;
    rc.reserve(d->m_enabledAttributes.size());
    for (auto attr: d->m_enabledAttributes) {
        rc << attr->id();
    }
    return rc;
}

void ApplicationsDataModel::setEnabledAttributes(const QStringList &enabledAttributes)
{
     beginResetModel();

     QVector<ProcessAttribute*> unusedAttributes = d->m_enabledAttributes;
     d->m_enabledAttributes.clear();

     for (auto attribute: enabledAttributes) {
         auto attr = d->m_availableAttributes[attribute];
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
             auto app = d->m_apps->getApplication(process->cGroup());
             if (!app) {
                 return;
             }
             const QModelIndex index = getQModelIndex(app, columnIndex);
             emit dataChanged(index, index);
         });

         attr->setEnabled(true);
     }

     for (auto unusedAttr: unusedAttributes) {
         disconnect(unusedAttr, &KSysGuard::ProcessAttribute::dataChanged, this, nullptr);
         unusedAttr->setEnabled(false);
     }

     endResetModel();

     emit enabledAttributesChanged();
}

QModelIndex ApplicationsDataModel::getQModelIndex(Application *application, int column) const
{
    Q_ASSERT(application);
    int row = d->m_apps->getAllApplications().indexOf(application);
    return index(row, column, QModelIndex());
}

QHash<int, QByteArray> ApplicationsDataModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    QMetaEnum e = ProcessDataModel::staticMetaObject.enumerator(ProcessDataModel::staticMetaObject.indexOfEnumerator("AdditionalRoles"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }

    return roles;
}

QVariant ApplicationsDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.column() > columnCount()) {
        return QVariant();
    }

    int attr = index.column();
    auto attribute = d->m_enabledAttributes[attr];
    switch(role) {
        case Qt::DisplayRole:
        case ProcessDataModel::FormattedValue: {
            KSysGuard::Application *app = reinterpret_cast< KSysGuard::Application* > (index.internalPointer());
            const QVariant value = attribute->appData(app);
            return KSysGuard::Formatter::formatValue(value, attribute->unit());
        }
        case ProcessDataModel::Value: {
            KSysGuard::Application *app = reinterpret_cast< KSysGuard::Application* > (index.internalPointer());
            const QVariant value = attribute->appData(app);
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
            return attribute->shortName();
        }
        case ProcessDataModel::Name: {
            return attribute->name();
        }
        case ProcessDataModel::Unit: {
            return attribute->unit();
        }
        case ProcessDataModel::PIDs: {
            KSysGuard::Application *app = reinterpret_cast< KSysGuard::Application* > (index.internalPointer());
            QVariantList pidList;
            std::transform(app->processes().constBegin(), app->processes().constEnd(), std::back_inserter(pidList), [](Process* process) -> QVariant {
                return QVariant::fromValue(process->pid());
            });
            return pidList;
        }
    }
    return QVariant();
}

QVariant ApplicationsDataModel::headerData(int section, Qt::Orientation orientation, int role) const
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
        return attribute->shortName();
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

ProcessAttributeModel *ApplicationsDataModel::attributesModel()
{
    //lazy load
    if (!d->m_attributeModel) {
        d->m_attributeModel = new KSysGuard::ProcessAttributeModel(d->m_processes, this);
    }
    return d->m_attributeModel;
}

bool ApplicationsDataModel::enabled()
{
    return d->m_updateTimer->isActive();
}

void ApplicationsDataModel::setEnabled(bool enabled)
{
    if (enabled) {
        d->m_updateTimer->start();
    } else {
        d->m_updateTimer->stop();
    }
}
