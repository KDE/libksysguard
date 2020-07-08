/*
    Copyright (C) 2020 Marco Martin <mart@kde.org>

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

#include "SensorGroup_p.h"

#include <QRegularExpression>
#include <QDebug>
#include <KLocalizedString>

namespace KSysGuard
{

SensorGroup::SensorGroup()
{
    retranslate();
}

SensorGroup::~SensorGroup()
{}

void SensorGroup::retranslate()
{
    m_sensorNames.clear();
    m_segmentNames.clear();

    m_sensorNames[QStringLiteral("cpu/cpu\\d+/TotalLoad")] = i18nc("Total load sensor of all cores", "[Group] CPU");
    m_sensorNames[QStringLiteral("cpu/cpu\\d+/user")] = i18nc("All cores user load sensors", "[Group] User Load");
    m_sensorNames[QStringLiteral("cpu/cpu\\d+/nice")] = i18nc("All cores nice load sensors", "[Group] Nice Load");
    m_sensorNames[QStringLiteral("cpu/cpu\\d+/sys")] = i18nc("All cores user system sensors", "[Group] System Load");
    m_sensorNames[QStringLiteral("cpu/cpu\\d+/idle")] = i18nc("All cores idle load sensors", "[Group] Idle Load");
    m_sensorNames[QStringLiteral("cpu/cpu\\d+/wait")] = i18nc("All cores wait load sensors", "[Group] Wait Load");
    m_sensorNames[QStringLiteral("cpu/cpu\\d+/clock")] = i18nc("All cores clock frequency sensors", "[Group] Clock Frequency");

    m_sensorNames[QStringLiteral("partitions/(?!all).*/usedspace")] = i18n("[Group] Used");
    m_sensorNames[QStringLiteral("partitions/(?!all).*/freespace")] = i18n("[Group] Available");
    m_sensorNames[QStringLiteral("partitions/(?!all).*/filllevel")] = i18n("[Group] Percentage Used");
    m_sensorNames[QStringLiteral("partitions/(?!all).*/total")] = i18n("[Group] Total Size");

    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/data")] = i18n("[Group] Received Data Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/dataTotal")] = i18n("[Group] Received Data");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/packets")] = i18n("[Group] Received Packets Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/packetsTotal")] = i18n("[Group] Received Packets");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/errors")] = i18n("[Group] Receiver Errors Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/errorsTotal")] = i18n("[Group] Receiver Errors");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/drops")] = i18n("[Group] Receiver Drops Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/dropsTotal")] = i18n("[Group] Receiver Drops");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/fifo")] = i18n("[Group] Receiver Fifo Overruns Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/fifoTotal")] = i18n("[Group] Receiver Fifo Overruns");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/frame")] = i18n("[Group] Receiver Frame Errors Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/frameTotal")] = i18n("[Group] Receiver Frame Errors");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/compressed")] = i18n("[Group] Compressed Packets Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/compressedTotal")] = i18n("[Group] Compressed Packets");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/multicast")] = i18n("[Group] Received Multicast Packets Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/receiver/multicastTotal")] = i18n("[Group] Multicast Packets");

    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/data")] = i18n("[Group] Sent Data Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/dataTotal")] = i18n("[Group] Sent Data ");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/packets")] = i18n("[Group] Sent Packets Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/packetsTotal")] = i18n("[Group] Sent Packets");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/errors")] = i18n("[Group] Transmitter Errors Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/errorsTotal")] = i18n("[Group] Transmitter Errors");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/drops")] = i18n("[Group] Transmitter Drops Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/dropsTotal")] = i18n("[Group] Transmitter Drops");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/fifo")] = i18n("[Group] Transmitter FIFO Overruns Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/fifoTotal")] = i18n("[Group] FIFO Overruns");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/collisions")] = i18n("[Group] Transmitter Collisions Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/collisionsTotal")] = i18n("[Group] Transmitter Collisions");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/carrier")] = i18n("[Group] Transmitter Carrier Losses Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/carrierTotal")] = i18n("[Group] Transmitter Carrier Losses");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/compressed")] = i18n("[Group] Transmitter Compressed Packets Rate");
    m_sensorNames[QStringLiteral("network/interfaces/(?!all).*/transmitter/compressedTotal")] = i18n("[Group] Transmitter Compressed Packets");



    m_segmentNames[QLatin1String("cpu\\d+")] = i18n("[Group] CPU");
    m_segmentNames[QLatin1String("disk\\d+")] = i18n("[Group] Disk");
    m_segmentNames[QLatin1String("(?!all).*")] = i18n("[Group]");
}

QString SensorGroup::groupRegexForId(const QString &key)
{

    QRegularExpression cpuExpr(QStringLiteral("cpu/cpu\\d+/(.*)"));
    QRegularExpression netRecExpr(QStringLiteral("network/interfaces/(?!all).*/receiver/(.*)"));
    QRegularExpression netTransExpr(QStringLiteral("network/interfaces/(?!all).*/transmitter/(.*)"));
    QRegularExpression partitionsExpr(QStringLiteral("partitions/(?!all).*/(.*)$"));

    if (key.contains(cpuExpr)) {
        QString expr = key;
        return expr.replace(cpuExpr, QStringLiteral("cpu/cpu\\d+/\\1"));

    } else if (key.contains(netRecExpr)) {
        QString expr = key;
        return expr.replace(netRecExpr, QStringLiteral("network/interfaces/(?!all).*/receiver/\\1"));

    } else if (key.contains(netTransExpr)) {
        QString expr = key;
        return expr.replace(netTransExpr, QStringLiteral("network/interfaces/(?!all).*/transmitter/\\1"));

    } else if (key.contains(partitionsExpr)) {
        QString expr = key;
        return expr.replace(partitionsExpr, QStringLiteral("partitions/(?!all).*/\\1"));
    }

    return QString();
}

QString SensorGroup::sensorNameForRegEx(const QString &expr)
{
    if (m_sensorNames.contains(expr)) {
        return m_sensorNames.value(expr);
    }

    return expr;
}

QString SensorGroup::segmentNameForRegEx(const QString &expr)
{
    if (m_segmentNames.contains(expr)) {
        return m_segmentNames.value(expr);
    }

    return expr;
}

} // namespace KSysGuard
