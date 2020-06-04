#pragma once

#include <QAbstractItemModel>

#include "process_attribute_model.h"

namespace KSysGuard {

class Application;

class Q_DECL_EXPORT ApplicationsDataModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList availableAttributes READ availableAttributes CONSTANT)
    Q_PROPERTY(QStringList enabledAttributes READ enabledAttributes WRITE setEnabledAttributes NOTIFY enabledAttributesChanged)
    Q_PROPERTY(QObject* attributesModel READ attributesModel CONSTANT)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    ApplicationsDataModel(QObject *parent = nullptr);
    ~ApplicationsDataModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    QStringList availableAttributes() const;
    QStringList enabledAttributes() const;
    void setEnabledAttributes(const QStringList &enabledAttributes);

    QModelIndex getQModelIndex(Application *process, int column) const;
    QHash<int, QByteArray> roleNames() const override;

    ProcessAttributeModel *attributesModel();

    bool enabled();
    void setEnabled(bool enabled);

Q_SIGNALS:
    void enabledAttributesChanged();
    void enabledChanged();

private:
    class Private;
    QScopedPointer<Private> d;
public:
};

}
