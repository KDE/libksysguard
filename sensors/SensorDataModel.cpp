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

#include <QMetaEnum>

#include "Formatter.h"
#include "SensorDaemonInterface_p.h"
#include "SensorDataModel.h"
#include "SensorInfo_p.h"
#include "sensors_logging.h"

using namespace KSysGuard;

class Q_DECL_HIDDEN SensorDataModel::Private
{
public:
    Private(SensorDataModel *qq)
        : q(qq)
    {
    }

    void sensorsChanged();
    void addSensor(const QString &id);
    void removeSensor(const QString &id);

    QStringList requestedSensors;

    QStringList sensors;
    QStringList objects;

    QHash<QString, SensorInfo> sensorInfos;
    QHash<QString, QVariant> sensorData;
    QVariantMap sensorColors;

    bool usedByQml = false;
    bool componentComplete = false;
    bool loaded = false;
    bool enabled = true;

private:
    SensorDataModel *q;
};

SensorDataModel::SensorDataModel(const QStringList &sensorIds, QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private(this))
{
    connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::sensorAdded, this, &SensorDataModel::onSensorAdded);
    connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::sensorRemoved, this, &SensorDataModel::onSensorRemoved);
    connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::metaDataChanged, this, &SensorDataModel::onMetaDataChanged);
    connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::valueChanged, this, &SensorDataModel::onValueChanged);

    // Empty string is used for entries that do not specify a wildcard object
    d->objects << QStringLiteral("");

    setSensors(sensorIds);
}

SensorDataModel::~SensorDataModel()
{
}

QHash<int, QByteArray> SensorDataModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("AdditionalRoles"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }

    return roles;
}

QVariant SensorDataModel::data(const QModelIndex &index, int role) const
{
    const bool check = checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::DoNotUseParent);
    if (!check) {
        return QVariant();
    }

    auto sensor = d->sensors.at(index.column());
    auto info = d->sensorInfos.value(sensor);
    auto data = d->sensorData.value(sensor);

    switch (role) {
    case Qt::DisplayRole:
    case FormattedValue:
        return Formatter::formatValue(data, info.unit);
    case Value:
        return data;
    case Unit:
        return info.unit;
    case Name:
        return info.name;
    case ShortName:
        if (info.shortName.isEmpty()) {
            return info.name;
        }
        return info.shortName;
    case Description:
        return info.description;
    case Minimum:
        return info.min;
    case Maximum:
        return info.max;
    case Type:
        return info.variantType;
    case SensorId:
        return sensor;
    case Color:
        if (!d->sensorColors.empty()) {
            return d->sensorColors.value(sensor);
        }
    case UpdateInterval:
        //TODO: Make this dynamic once the backend supports it.
        return BackendUpdateInterval;
    default:
        break;
    }

    return QVariant();
}

QVariant SensorDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) {
        return QVariant();
    }

    if (section < 0 || section >= d->sensors.size()) {
        return QVariant();
    }

    auto sensor = d->sensors.at(section);
    auto info = d->sensorInfos.value(sensor);

    switch (role) {
    case Qt::DisplayRole:
    case ShortName:
        if (info.shortName.isEmpty()) {
            return info.name;
        }
        return info.shortName;
    case Name:
        return info.name;
    case SensorId:
        return sensor;
    case Unit:
        return info.unit;
    case Description:
        return info.description;
    case Minimum:
        return info.min;
    case Maximum:
        return info.max;
    case Type:
        return info.variantType;
    case UpdateInterval:
        //TODO: Make this dynamic once the backend supports it.
        return BackendUpdateInterval;
    default:
        break;
    }

    return QVariant();
}

int SensorDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->objects.count();
}

int SensorDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->sensors.count();
}

qreal SensorDataModel::minimum() const
{
    if (d->sensors.isEmpty()) {
        return 0;
    }

    auto result = std::min_element(d->sensorInfos.cbegin(), d->sensorInfos.cend(), [](const SensorInfo &first, const SensorInfo &second) { return first.min < second.min; });
    return (*result).min;
}

qreal SensorDataModel::maximum() const
{
    if (d->sensors.isEmpty()) {
        return 0;
    }

    auto result = std::max_element(d->sensorInfos.cbegin(), d->sensorInfos.cend(), [](const SensorInfo &first, const SensorInfo &second) { return first.max < second.max; });
    return (*result).max;
}

QStringList SensorDataModel::sensors() const
{
    return d->requestedSensors;
}

void SensorDataModel::setSensors(const QStringList &sensorIds)
{
    if (d->requestedSensors == sensorIds) {
        return;
    }

    d->requestedSensors = sensorIds;

    if (!d->usedByQml || d->componentComplete) {
        d->sensorsChanged();
    }
    Q_EMIT readyChanged();
    Q_EMIT sensorsChanged();
}

bool SensorDataModel::enabled() const
{
    return d->enabled;
}

void SensorDataModel::setEnabled(bool newEnabled)
{
    if (newEnabled == d->enabled) {
        return;
    }

    d->enabled = newEnabled;
    if (d->enabled) {
        SensorDaemonInterface::instance()->subscribe(d->sensorInfos.keys());
        SensorDaemonInterface::instance()->requestMetaData(d->sensorInfos.keys());
    } else {
        SensorDaemonInterface::instance()->unsubscribe(d->sensorInfos.keys());
    }

    Q_EMIT enabledChanged();
}

QVariantMap SensorDataModel::sensorColors() const
{
    return d->sensorColors;
}

void SensorDataModel::setSensorColors(const QVariantMap &sensorColors)
{
    if (sensorColors == d->sensorColors) {
        return;
    }
    d->sensorColors = sensorColors;
    Q_EMIT sensorColorsChanged();
}

bool KSysGuard::SensorDataModel::isReady() const
{
    return d->sensors.size() == d->requestedSensors.size();
}

void SensorDataModel::addSensor(const QString &sensorId)
{
    d->addSensor(sensorId);
}

void SensorDataModel::removeSensor(const QString &sensorId)
{
    d->removeSensor(sensorId);
}

int KSysGuard::SensorDataModel::column(const QString &sensorId) const
{
    return d->sensors.indexOf(sensorId);
}

void KSysGuard::SensorDataModel::classBegin()
{
    d->usedByQml = true;
}

void KSysGuard::SensorDataModel::componentComplete()
{
    d->componentComplete = true;

    d->sensorsChanged();

    emit sensorsChanged();
}

void SensorDataModel::Private::addSensor(const QString &id)
{
    if (!requestedSensors.contains(id)) {
        return;
    }

    qCDebug(LIBKSYSGUARD_SENSORS) << "Add Sensor" << id;

    sensors.append(id);
    SensorDaemonInterface::instance()->subscribe(id);
    SensorDaemonInterface::instance()->requestMetaData(id);
}

void SensorDataModel::Private::removeSensor(const QString &id)
{
    const int col = sensors.indexOf(id);
    if (col == -1) {
        return;
    }

    q->beginRemoveColumns(QModelIndex(), col, col);

    sensors.removeAt(col);
    sensorInfos.remove(id);
    sensorData.remove(id);

    q->endRemoveColumns();
}

void SensorDataModel::onSensorAdded(const QString &sensorId)
{
    if (!d->enabled) {
        return;
    }

    d->addSensor(sensorId);
}

void SensorDataModel::onSensorRemoved(const QString &sensorId)
{
    if (!d->enabled) {
        return;
    }

    d->removeSensor(sensorId);
}

void SensorDataModel::onMetaDataChanged(const QString &sensorId, const SensorInfo &info)
{
    if (!d->enabled) {
        return;
    }

    auto column = d->sensors.indexOf(sensorId);
    if (column == -1) {
        return;
    }

    qCDebug(LIBKSYSGUARD_SENSORS) << "Received metadata change for" << sensorId;

    // Simple case: Just an update for a sensor's metadata
    if (d->sensorInfos.contains(sensorId)) {
        d->sensorInfos[sensorId] = info;
        Q_EMIT dataChanged(index(0, column), index(0, column), {Qt::DisplayRole, Name, ShortName, Description, Unit, Minimum, Maximum, Type, FormattedValue});
        Q_EMIT sensorMetaDataChanged();
        return;
    }

    // Otherwise, it's a new sensor that was added
    beginInsertColumns(QModelIndex{}, column, column);
    d->sensorInfos[sensorId] = info;
    d->sensorData[sensorId] = QVariant{};
    endInsertColumns();

    SensorDaemonInterface::instance()->requestValue(sensorId);
    emit sensorMetaDataChanged();
    Q_EMIT readyChanged();
}

void SensorDataModel::onValueChanged(const QString &sensorId, const QVariant &value)
{
    const auto column = d->sensors.indexOf(sensorId);
    if (column == -1 || !d->enabled) {
        return;
    }

    d->sensorData[sensorId] = value;
    Q_EMIT dataChanged(index(0, column), index(0, column), {Qt::DisplayRole, Value, FormattedValue});
}

void SensorDataModel::Private::sensorsChanged()
{
    q->beginResetModel();

    SensorDaemonInterface::instance()->unsubscribe(sensors);

    sensors.clear();
    sensorData.clear();
    sensorInfos.clear();

    sensors = requestedSensors;

    for (const auto& sensor : sensors) {
        sensorInfos[sensor] = KSysGuard::SensorInfo();
    }

    SensorDaemonInterface::instance()->subscribe(requestedSensors);
    SensorDaemonInterface::instance()->requestMetaData(requestedSensors);

    q->endResetModel();
}
