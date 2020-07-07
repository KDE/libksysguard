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

#include <QRegularExpression>
#include <QDebug>
#include <KLocalizedString>

namespace KSysGuard
{

QString groupRegexForId(const QString &key)
{

    QRegularExpression cpuExpr(QStringLiteral("cpu/cpu\\d+/(.*)"));
    QRegularExpression netRecExpr(QStringLiteral("network/interfaces/.*/receiver/(.*)"));
    QRegularExpression netTransExpr(QStringLiteral("network/interfaces/.*/transmitter/(.*)"));
    QRegularExpression partitionsExpr(QStringLiteral("partitions/.*/(.*)$"));

    if (key.contains(cpuExpr)) {
        QString expr = key;
        return expr.replace(cpuExpr, QStringLiteral("cpu/cpu\\d+/\\1"));

    } else if (key.contains(netRecExpr)) {
        QString expr = key;
        return expr.replace(netRecExpr, QStringLiteral("network/interfaces/.*/receiver/\\1"));

    } else if (key.contains(netTransExpr)) {
        QString expr = key;
        return expr.replace(netTransExpr, QStringLiteral("network/interfaces/.*/transmitter/\\1"));

    } else if (key.contains(partitionsExpr)) {
        QString expr = key;
        return expr.replace(partitionsExpr, QStringLiteral("partitions/.*/\\1"));
    }

    return QString();
}

QString sensorNameForRegEx(const QString &expr)
{
    if (expr == QStringLiteral("cpu/cpu\\d+/TotalLoad")) {
        return i18nc("Total load sensor of all cores", "[Group] CPU (%)");
    } else if (expr == QStringLiteral("cpu/cpu\\d+/user")) {
        return i18nc("All cores user load sensors", "[Group] User Load (%)");
    } else if (expr == QStringLiteral("cpu/cpu\\d+/nice")) {
        return i18nc("All cores nice load sensors", "[Group] Nice Load (%)");
    } else if (expr == QStringLiteral("cpu/cpu\\d+/sys")) {
        return i18nc("All cores user system sensors", "[Group] System Load (%)");
    } else if (expr == QStringLiteral("cpu/cpu\\d+/idle")) {
        return i18nc("All cores idle load sensors", "[Group] Idle Load (%)");
    } else if (expr == QStringLiteral("cpu/cpu\\d+/wait")) {
        return i18nc("All cores wait load sensors", "[Group] Wait Load (%)");
    } else if (expr == QStringLiteral("cpu/cpu\\d+/clock")) {
        return i18nc("All cores clock frequency sensors", "[Group] Clock Frequency (MHz)");

    } else if (expr == QStringLiteral("partitions/.*/usedspace")) {
        return i18n("[Group] Used (KiB)");
    } else if (expr == QStringLiteral("partitions/.*/freespace")) {
        return i18n("[Group] Available (KiB)");
    } else if (expr == QStringLiteral("partitions/.*/filllevel")) {
        return i18n("[Group] Percentage Used (%)");
    } else if (expr == QStringLiteral("partitions/.*/total")) {
        return i18n("[Group] Total Size (KiB)");

    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/data")) {
        return i18n("[Group] Received Data Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/dataTotal")) {
        return i18n("[Group] Received Data");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/packets")) {
        return i18n("[Group] Received Packets Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/packetsTotal")) {
        return i18n("[Group] Received Packets");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/errors")) {
        return i18n("[Group] Receiver Errors Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/errorsTotal")) {
        return i18n("[Group] Receiver Errors");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/drops")) {
        return i18n("[Group] Receiver Drops Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/dropsTotal")) {
        return i18n("[Group] Receiver Drops");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/fifo")) {
        return i18n("[Group] Receiver Fifo Overruns Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/fifoTotal")) {
        return i18n("[Group] Receiver Fifo Overruns");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/frame")) {
        return i18n("[Group] Receiver Frame Errors Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/frameTotal")) {
        return i18n("[Group] Receiver Frame Errors");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/compressed")) {
        return i18n("[Group] Compressed Packets Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/compressedTotal")) {
        return i18n("[Group] Compressed Packets");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/multicast")) {
        return i18n("[Group] Received Multicast Packets Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/receiver/multicastTotal")) {
        return i18n("[Group] Multicast Packets");

    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/data")) {
        return i18n("[Group] Sent Data Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/dataTotal")) {
        return i18n("[Group] Sent Data ");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/packets")) {
        return i18n("[Group] Sent Packets Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/packetsTotal")) {
        return i18n("[Group] Sent Packets");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/errors")) {
        return i18n("[Group] Transmitter Errors Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/errorsTotal")) {
        return i18n("[Group] Transmitter Errors");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/drops")) {
        return i18n("[Group] Transmitter Drops Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/dropsTotal")) {
        return i18n("[Group] Transmitter Drops");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/fifo")) {
        return i18n("[Group] Transmitter FIFO Overruns Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/fifoTotal")) {
        return i18n("[Group] FIFO Overruns");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/collisions")) {
        return i18n("[Group] Transmitter Collisions Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/collisionsTotal")) {
        return i18n("[Group] Transmitter Collisions");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/carrier")) {
        return i18n("[Group] Transmitter Carrier Losses Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/carrierTotal")) {
        return i18n("[Group] Transmitter Carrier Losses");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/compressed")) {
        return i18n("[Group] Transmitter Compressed Packets Rate");
    } else if (expr == QStringLiteral("network/interfaces/.*/transmitter/compressedTotal")) {
        return i18n("[Group] Transmitter Compressed Packets");
    }

    return QString();
}

QString segmentNameForRegEx(const QString &expr)
{
    if (expr == QLatin1String("cpu\\d+")) {
        return i18n("[Group] CPU");
    } else if (expr == QLatin1String("disk\\d+")) {
        return i18n("[Group] Disk");
    } else if (expr == QLatin1String(".*")) {
        return i18n("[Group]");
    }

    return expr;
}

} // namespace KSysGuard
