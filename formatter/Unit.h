/*
    Copyright (C) 2019 Vlad Zahorodnii <vladzzag@gmail.com>

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
    UnitWattHour
};
Q_ENUM_NS(Unit)

} // namespace KSysGuard
