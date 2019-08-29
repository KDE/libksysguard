#pragma once

#include <QAbstractItemModel>


namespace KSysGuard {

class Process;
class ProcessDataModelPrivate;

class ProcessDataModel : public QAbstractItemModel
{
    enum AdditionalRoles {
        PlainValue = Qt::UserRole + 1,
        PlainValueHistory,
        Timestamp,
        Entity,
        Unit,
        Name,
        ShortName,
    };
    Q_ENUM(AdditionalRoles)

public:
    ProcessDataModel(QObject *parent = nullptr);
    ~ProcessDataModel() override;
    QHash<int, QByteArray> roleNames() const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    QStringList enabledSensors();
    void setEnabledSensors();

    QModelIndex getQModelIndex(Process *process, int column) const;

private:
    QScopedPointer<ProcessDataModelPrivate> d;
    friend class ProcessDataModelPrivate;
};

}
