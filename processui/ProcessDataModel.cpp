#include "ProcessDataModel.h"
#include "processcore/extended_process_list.h"
#include "processcore/formatter.h"
#include "processcore/process.h"
#include "processcore/process_attribute.h"
#include "processcore/process_data_provider.h"

#include <QMetaEnum>

using namespace KSysGuard;

class KSysGuard::ProcessDataModelPrivate: public QObject
{
public:
    ProcessDataModelPrivate(ProcessDataModel *q);
    void beginInsertRow( KSysGuard::Process *parent);
    /** Called from KSysGuard::Processes
        *  We have finished inserting a process
        */
    void endInsertRow();
    /** Called from KSysGuard::Processes
        *  This indicates we are about to remove a process in the model.  Emit the appropriate signals
        */
    void beginRemoveRow( KSysGuard::Process *process);
    /** Called from KSysGuard::Processes
        *  We have finished removing a process
        */
    void endRemoveRow();
    /** Called from KSysGuard::Processes
        *  This indicates we are about to move a process in the model from one parent process to another.  Emit the appropriate signals
        */
    void beginMoveProcess(KSysGuard::Process *process, KSysGuard::Process *new_parent);
    /** Called from KSysGuard::Processes
        *  We have finished moving a process
        */
    void endMoveRow();

    KSysGuard::ExtendedProcesses *m_processes;
    QVector<KSysGuard::ProcessAttribute* > m_extraAttributes;
    ProcessDataModel *q;
};

ProcessDataModel::ProcessDataModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new ProcessDataModelPrivate(this))
{}

ProcessDataModel::~ProcessDataModel()
{}

ProcessDataModelPrivate::ProcessDataModelPrivate(ProcessDataModel *_q):
    q(_q)
{

    m_processes = new KSysGuard::ExtendedProcesses(this);

//     connect( m_processes, &KSysGuard::Processes::processChanged, this, &ProcessDataModelPrivate::processChanged);
    connect( m_processes, &KSysGuard::Processes::beginAddProcess, this, &ProcessDataModelPrivate::beginInsertRow);
    connect( m_processes, &KSysGuard::Processes::endAddProcess, this, &ProcessDataModelPrivate::endInsertRow);
    connect( m_processes, &KSysGuard::Processes::beginRemoveProcess, this, &ProcessDataModelPrivate::beginRemoveRow);
    connect( m_processes, &KSysGuard::Processes::endRemoveProcess, this, &ProcessDataModelPrivate::endRemoveRow);
    connect( m_processes, &KSysGuard::Processes::beginMoveProcess, this,
            &ProcessDataModelPrivate::beginMoveProcess);
    connect( m_processes, &KSysGuard::Processes::endMoveProcess, this, &ProcessDataModelPrivate::endMoveRow);

    m_extraAttributes = m_processes->attributes();
    for (int i = 0 ; i < m_extraAttributes.count(); i ++) {
        m_extraAttributes[i]->setEnabled(true); // In future we will toggle this based on column visibility

        connect(m_extraAttributes[i], &KSysGuard::ProcessAttribute::dataChanged, this, [this, i](KSysGuard::Process *process) {
            const QModelIndex index = q->getQModelIndex(process, i);
            emit q->dataChanged(index, index);
        });
    }
}

QVariant ProcessDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.column() > columnCount()) {
        return QVariant();
    }

    int attr = index.column();
    switch (role) {
        case ProcessDataModel::PlainValue: {
            KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
            const QVariant value = d->m_extraAttributes[attr]->data(process);
            return value;
        }
        case Qt::DisplayRole: {
            KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
            const QVariant value = d->m_extraAttributes[attr]->data(process);
            return KSysGuard::Formatter::formatValue(value, d->m_extraAttributes[attr]->unit());
        }
        case Qt::TextAlignmentRole: {
            KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
            const QVariant value = d->m_extraAttributes[attr]->data(process);
            if (value.canConvert(QMetaType::LongLong)
                && static_cast<QMetaType::Type>(value.type()) != QMetaType::QString) {
                return Qt::AlignRight + Qt::AlignVCenter;
            }
            return Qt::AlignLeft + Qt::AlignVCenter;
        }
        }
        return QVariant();
    }

int ProcessDataModel::rowCount(const QModelIndex &parent) const
{
        if(parent.isValid()) return 0; //In flat mode, none of the processes have children
        return d->m_processes->processCount();
}

QModelIndex ProcessDataModel::parent ( const QModelIndex & index ) const
{
    return QModelIndex();
}

void ProcessDataModelPrivate::beginInsertRow( KSysGuard::Process *process)
{
    Q_ASSERT(process);
    int row = m_processes->processCount();
    q->beginInsertRows( QModelIndex(), row, row );
}

void ProcessDataModelPrivate::endInsertRow()
{
    q->endInsertRows();
}

void ProcessDataModelPrivate::beginRemoveRow( KSysGuard::Process *process )
{
    return q->beginRemoveRows(QModelIndex(), process->index(), process->index());
}

void ProcessDataModelPrivate::endRemoveRow()
{
    q->endRemoveRows();
}

void ProcessDataModelPrivate::beginMoveProcess(KSysGuard::Process *process, KSysGuard::Process *new_parent)
{
}

void ProcessDataModelPrivate::endMoveRow()
{
}

QModelIndex ProcessDataModel::getQModelIndex( KSysGuard::Process *process, int column) const
{
    Q_ASSERT(process);
    int pid = process->pid();
    if (pid == -1) return QModelIndex(); //pid -1 is our fake process meaning the very root (never drawn).  To represent that, we return QModelIndex() which also means the top element
    int row = 0;
//     if(d->mSimple) {
        row = process->index();
//     } else {
//         row = process->parent()->children().indexOf(process);
//     }
    Q_ASSERT(row != -1);
    return createIndex(row, column, process);
}

int ProcessDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->m_extraAttributes.count();
}

QHash<int, QByteArray> ProcessDataModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("AdditionalRoles"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }

    roles[Qt::TextAlignmentRole] = "alignment";

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

    auto attribute = d->m_extraAttributes[section];

    switch (role) {
    case Qt::DisplayRole: {
        return attribute->shortName();
    }
    case PlainValue: {
        return attribute->id();
    }
    case Qt::TextAlignmentRole: {
        switch (attribute->unit()) {
            case KSysGuard::UnitByte:
            case KSysGuard::UnitKiloByte:
            case KSysGuard::UnitMegaByte:
            case KSysGuard::UnitGigaByte:
            case KSysGuard::UnitTeraByte:
            case KSysGuard::UnitPetaByte:
            case KSysGuard::UnitByteRate:
            case KSysGuard::UnitKiloByteRate:
            case KSysGuard::UnitMegaByteRate:
            case KSysGuard::UnitGigaByteRate:
            case KSysGuard::UnitTeraByteRate:
            case KSysGuard::UnitPetaByteRate:
            case KSysGuard::UnitHertz:
            case KSysGuard::UnitKiloHertz:
            case KSysGuard::UnitMegaHertz:
            case KSysGuard::UnitGigaHertz:
            case KSysGuard::UnitTeraHertz:
            case KSysGuard::UnitPetaHertz:
            case KSysGuard::UnitPercent:
            case KSysGuard::UnitRate:
                return Qt::AlignCenter;
            default: break;
        }

        return Qt::AlignLeft;
    }
    case Unit: {
//         const SensorInfo &info = d->sensorManager->sensorInfo(attribute);
//         return info.unit;
    }
    default:
        break;
    }

    return QVariant();
}
