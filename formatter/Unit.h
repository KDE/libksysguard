/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

// Qt
#include <QMetaType>

#include "formatter_export.h"

namespace KSysGuard
{
FORMATTER_EXPORT Q_NAMESPACE

    /**
     * This enum type is used to specify metric prefixes.
     */
    enum MetricPrefix {
        MetricPrefixAutoAdjust = -1,
        MetricPrefixUnity = 0,
        MetricPrefixKilo,
        MetricPrefixMega,
        MetricPrefixGiga,
        MetricPrefixTera,
        MetricPrefixPeta,
        MetricPrefixLast = MetricPrefixPeta
    };
Q_ENUM_NS(MetricPrefix)

/**
 * This enum types is used to specify units.
 */
enum Unit {
    UnitInvalid = -1,
    UnitNone = 0,

    // Byte size units.
    UnitByte = 100,
    UnitKiloByte = MetricPrefixKilo + UnitByte,
    UnitMegaByte = MetricPrefixMega + UnitByte,
    UnitGigaByte = MetricPrefixGiga + UnitByte,
    UnitTeraByte = MetricPrefixTera + UnitByte,
    UnitPetaByte = MetricPrefixPeta + UnitByte,

    // Data rate units.
    UnitByteRate = 200,
    UnitKiloByteRate = MetricPrefixKilo + UnitByteRate,
    UnitMegaByteRate = MetricPrefixMega + UnitByteRate,
    UnitGigaByteRate = MetricPrefixGiga + UnitByteRate,
    UnitTeraByteRate = MetricPrefixTera + UnitByteRate,
    UnitPetaByteRate = MetricPrefixPeta + UnitByteRate,

    // Frequency.
    UnitHertz = 300,
    UnitKiloHertz = MetricPrefixKilo + UnitHertz,
    UnitMegaHertz = MetricPrefixMega + UnitHertz,
    UnitGigaHertz = MetricPrefixGiga + UnitHertz,
    UnitTeraHertz = MetricPrefixTera + UnitHertz,
    UnitPetaHertz = MetricPrefixPeta + UnitHertz,

    // Time units.
    UnitBootTimestamp = 400, // deprecated
    UnitSecond,
    UnitTime,

    // Misc units.
    UnitCelsius = 500,
    UnitDecibelMilliWatts,
    UnitPercent,
    UnitRate,
    UnitRpm,
    UnitVolt,
    UnitWatt,
    UnitWattHour,
    UnitAmpere
};
Q_ENUM_NS(Unit)

} // namespace KSysGuard
