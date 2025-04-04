/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SensorGroup_p.h"

#include <KLocalizedString>
#include <QDebug>
#include <QRegularExpression>

using namespace Qt::StringLiterals;

namespace KSysGuard
{
SensorGroup::SensorGroup()
{
    retranslate();
}

SensorGroup::~SensorGroup()
{
}

void SensorGroup::retranslate()
{
    m_sensorNames.clear();
    m_segmentNames.clear();

    m_sensorNames[u"cpu/cpu\\d+/usage"_s] = i18nc("Total load sensor of all cores", "[Group] Total Usage");
    m_sensorNames[u"cpu/cpu\\d+/user"_s] = i18nc("All cores user load sensors", "[Group] User Usage");
    m_sensorNames[u"cpu/cpu\\d+/system"_s] = i18nc("All cores user system sensors", "[Group] System Usage");
    m_sensorNames[u"cpu/cpu\\d+/wait"_s] = i18nc("All cores wait load sensors", "[Group] Wait Usage");
    m_sensorNames[u"cpu/cpu\\d+/frequency"_s] = i18nc("All cores clock frequency sensors", "[Group] Clock Frequency");
    m_sensorNames[u"cpu/cpu\\d+/temperature"_s] = i18nc("All cores temperature sensors", "[Group] Temperature");
    m_sensorNames[u"cpu/cpu\\d+/name"_s] = i18nc("All cores names", "[Group] Name");

    m_sensorNames[u"gpu/gpu\\d+/usage"_s] = i18nc("GPU group sensor", "[Group] Total Usage");
    m_sensorNames[u"gpu/gpu\\d+/usedVram"_s] = i18nc("GPU group sensor", "[Group] Video Memory Used");
    m_sensorNames[u"gpu/gpu\\d+/totalVram"_s] = i18nc("GPU group sensor", "[Group] Total Video Memory");
    m_sensorNames[u"gpu/gpu\\d+/name"_s] = i18nc("GPU group sensor", "[Group] Name");

    m_sensorNames[u"partitions/(?!all).*/usedspace"_s] = i18n("[Group] Used");
    m_sensorNames[u"partitions/(?!all).*/freespace"_s] = i18n("[Group] Available");
    m_sensorNames[u"partitions/(?!all).*/filllevel"_s] = i18n("[Group] Percentage Used");
    m_sensorNames[u"partitions/(?!all).*/total"_s] = i18n("[Group] Total Size");

    m_sensorNames[u"network/(?!all).*/network"_s] = i18n("[Group] Network Name");
    m_sensorNames[u"network/(?!all).*/ipv4address"_s] = i18n("[Group] IPv4 Address");
    m_sensorNames[u"network/(?!all).*/ipv6address"_s] = i18n("[Group] IPv6 Address");
    m_sensorNames[u"network/(?!all).*/signal"_s] = i18n("[Group] Signal Strength");
    m_sensorNames[u"network/(?!all).*/download"_s] = i18n("[Group] Download Rate");
    m_sensorNames[u"network/(?!all).*/downloadBits"_s] = i18n("[Group] Download Rate");
    m_sensorNames[u"network/(?!all).*/totalDownload"_s] = i18n("[Group] Total Downloaded");
    m_sensorNames[u"network/(?!all).*/upload"_s] = i18n("[Group] Upload Rate");
    m_sensorNames[u"network/(?!all).*/uploadBits"_s] = i18n("[Group] Upload Rate");
    m_sensorNames[u"network/(?!all).*/totalUpload"_s] = i18n("[Group] Total Uploaded");

    m_sensorNames[u"disk/(?!all).*/name"_s] = i18n("[Group] Name");
    m_sensorNames[u"disk/(?!all).*/total"_s] = i18n("[Group] Total Space");
    m_sensorNames[u"disk/(?!all).*/used"_s] = i18n("[Group] Used Space");
    m_sensorNames[u"disk/(?!all).*/free"_s] = i18n("[Group] Free Space");
    m_sensorNames[u"disk/(?!all).*/read"_s] = i18n("[Group] Read Rate");
    m_sensorNames[u"disk/(?!all).*/write"_s] = i18n("[Group] Write Rate");
    m_sensorNames[u"disk/(?!all).*/usedPercent"_s] = i18n("[Group] Percentage Used");
    m_sensorNames[u"disk/(?!all).*/freePercent"_s] = i18n("[Group] Percentage Free");

    m_sensorNames[u"power/(?!all).*/name"_s] = i18nc("Power group sensor", "[Group] Name");
    m_sensorNames[u"power/(?!all).*/charge"_s] = i18nc("Power group sensor", "[Group] Charge");
    m_sensorNames[u"power/(?!all).*/chargePercentage"_s] = i18nc("Power group sensor", "[Group] Charge Percentage");
    m_sensorNames[u"power/(?!all).*/chargeRate"_s] = i18nc("Power group sensor", "[Group] Charging Rate");
    m_sensorNames[u"power/(?!all).*/capacity"_s] = i18nc("Power group sensor", "[Group] Current Capacity");
    m_sensorNames[u"power/(?!all).*/design"_s] = i18nc("Power group sensor", "[Group] Design Capacity");
    m_sensorNames[u"power/(?!all).*/health"_s] = i18nc("Power group sensor", "[Group] Health");

    m_segmentNames[u"cpu/cpu\\d+"_s] = i18n("[Group] CPU");
    m_segmentNames[u"gpu/gpu\\d+"_s] = i18n("[Group] GPU");
    m_segmentNames[u"network/(?!all).*"_s] = i18nc("Network group title", "[Group] Network Device");
    m_segmentNames[u"disk/(?!all).*"_s] = i18n("[Group] Disk");
    m_segmentNames[u"power/(?!all).*"_s] = i18nc("Power group title", "[Group] Power");
}

QString SensorGroup::groupRegexForId(QString key /*Intentional copy*/)
{
    static const QRegularExpression cpuExpr(u"cpu/cpu\\d+/(.*)"_s);
    static const QRegularExpression gpuExpr(u"gpu/gpu\\d+/(.*)"_s);
    static const QRegularExpression netExpr(u"network/(?!all).*/(.*)$"_s);
    static const QRegularExpression partitionsExpr(u"partitions/(?!all).*/(.*)$"_s);
    static const QRegularExpression diskExpr(u"disk/(?!all).*/(.*)"_s);
    static const QRegularExpression powerExpr(u"power/(?!all).*/(.*)"_s);

    if (key.contains(cpuExpr)) {
        key.replace(cpuExpr, u"cpu/cpu\\d+/\\1"_s);
    } else if (key.contains(gpuExpr)) {
        key.replace(gpuExpr, u"gpu/gpu\\d+/\\1"_s);
    } else if (key.contains(netExpr)) {
        key.replace(netExpr, u"network/(?!all).*/\\1"_s);
    } else if (key.contains(partitionsExpr)) {
        key.replace(partitionsExpr, u"partitions/(?!all).*/\\1"_s);
    } else if (key.contains(diskExpr)) {
        key.replace(diskExpr, u"disk/(?!all).*/\\1"_s);
    } else if (key.contains(powerExpr)) {
        key.replace(powerExpr, u"power/(?!all).*/\\1"_s);
    } else {
        return QString{};
    }

    // Only return the result if we actually have a proper name for it.
    // This avoids us showing unmapped raw regex in the sensor tree model.
    if (m_sensorNames.contains(key)) {
        return key;
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
