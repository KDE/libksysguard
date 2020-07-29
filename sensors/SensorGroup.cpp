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

    m_sensorNames[QStringLiteral("network/(?!all).*/network")] = i18n("[Group] Network Name");
    m_sensorNames[QStringLiteral("network/(?!all).*/ipv4address")] = i18n("[Group] IPv4 Address");
    m_sensorNames[QStringLiteral("network/(?!all).*/ipv6address")] = i18n("[Group] IPv6 Address");
    m_sensorNames[QStringLiteral("network/(?!all).*/signal")] = i18n("[Group] Signal Strength");
    m_sensorNames[QStringLiteral("network/(?!all).*/download")] = i18n("[Group] Download Rate");
    m_sensorNames[QStringLiteral("network/(?!all).*/totalDownload")] = i18n("[Group] Total Downloaded");
    m_sensorNames[QStringLiteral("network/(?!all).*/upload")] = i18n("[Group] Upload Rate");
    m_sensorNames[QStringLiteral("network/(?!all).*/totalUpload")] = i18n("[Group] Total Uploaded");

    m_segmentNames[QLatin1String("cpu\\d+")] = i18n("[Group] CPU");
    m_segmentNames[QLatin1String("disk\\d+")] = i18n("[Group] Disk");
    m_segmentNames[QLatin1String("(?!all).*")] = i18n("[Group]");
}

QString SensorGroup::groupRegexForId(const QString &key)
{

    QRegularExpression cpuExpr(QStringLiteral("cpu/cpu\\d+/(.*)"));
    QRegularExpression netExpr(QStringLiteral("network/(?!all).*/(.*)$"));
    QRegularExpression partitionsExpr(QStringLiteral("partitions/(?!all).*/(.*)$"));

    if (key.contains(cpuExpr)) {
        QString expr = key;
        return expr.replace(cpuExpr, QStringLiteral("cpu/cpu\\d+/\\1"));

    } else if (key.contains(netExpr)) {
        QString expr = key;
        return expr.replace(netExpr, QStringLiteral("network/(?!all).*/\\1"));

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
