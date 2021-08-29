/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemsta@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <functional>
#include <memory>

#include <QPointer>
#include <QRegularExpression>
#include <QVariant>
#include <QVector>

#include "SensorObject.h"
#include "SensorPlugin.h"
#include "SensorProperty.h"

#include "systemstats_export.h"

namespace KSysGuard
{
/**
 * @todo write docs
 */
class SYSTEMSTATS_EXPORT AggregateSensor : public SensorProperty
{
    Q_OBJECT

public:
    AggregateSensor(SensorObject *provider, const QString &id, const QString &name);
    ~AggregateSensor();

    QVariant value() const override;
    void subscribe() override;
    void unsubscribe() override;

    QRegularExpression matchSensors() const;
    void setMatchSensors(const QRegularExpression &objectMatch, const QString &propertyId);
    std::function<QVariant(QVariant, QVariant)> aggregateFunction() const;
    void setAggregateFunction(const std::function<QVariant(QVariant, QVariant)> &function);

    void addSensor(SensorProperty *sensor);
    void removeSensor(const QString &sensorPath);

    int matchCount() const;

private:
    void updateSensors();
    void sensorDataChanged(SensorProperty *sensor);
    void delayedEmitDataChanged();

    class Private;
    const std::unique_ptr<Private> d;
};

class Q_DECL_EXPORT PercentageSensor : public SensorProperty
{
    Q_OBJECT
public:
    PercentageSensor(SensorObject *provider, const QString &id, const QString &name);
    ~PercentageSensor() override;

    QVariant value() const override;
    void subscribe() override;
    void unsubscribe() override;

    void setBaseSensor(SensorProperty *sensor);

private:
    class Private;
    const std::unique_ptr<Private> d;
};

} // namespace KSysGuard
