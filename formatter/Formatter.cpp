/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    formatBootTimestamp is based on TimeUtil class:
    SPDX-FileCopyrightText: 2014 Gregor Mi <codestruct@posteo.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "Formatter.h"

#include <KLocalizedString>

#include <QFontMetrics>
#include <QLocale>
#include <QTime>

#include <cmath>

#ifdef Q_OS_OSX
#include <mach/clock.h>
#include <mach/mach.h>
#else
#include <ctime>
#endif

#include <unistd.h>

#include "formatter_debug.h"

namespace KSysGuard
{
// TODO: Is there a bit nicer way to handle formatting?

static KLocalizedString unitFormat(Unit unit)
{
    const static KLocalizedString B = ki18nc("Bytes unit symbol", "%1 B");
    const static KLocalizedString KiB = ki18nc("Kilobytes unit symbol", "%1 KiB");
    const static KLocalizedString MiB = ki18nc("Megabytes unit symbol", "%1 MiB");
    const static KLocalizedString GiB = ki18nc("Gigabytes unit symbol", "%1 GiB");
    const static KLocalizedString TiB = ki18nc("Terabytes unit symbol", "%1 TiB");
    const static KLocalizedString PiB = ki18nc("Petabytes unit symbol", "%1 PiB");

    const static KLocalizedString bps = ki18nc("Bytes per second unit symbol", "%1 B/s");
    const static KLocalizedString Kbps = ki18nc("Kilobytes per second unit symbol", "%1 KiB/s");
    const static KLocalizedString Mbps = ki18nc("Megabytes per second unit symbol", "%1 MiB/s");
    const static KLocalizedString Gbps = ki18nc("Gigabytes per second unit symbol", "%1 GiB/s");
    const static KLocalizedString Tbps = ki18nc("Gigabytes per second unit symbol", "%1 TiB/s");
    const static KLocalizedString Pbps = ki18nc("Gigabytes per second unit symbol", "%1 PiB/s");

    const static KLocalizedString Hz = ki18nc("Hertz unit symbol", "%1 Hz");
    const static KLocalizedString kHz = ki18nc("Kilohertz unit symbol", "%1 kHz");
    const static KLocalizedString MHz = ki18nc("Megahertz unit symbol", "%1 MHz");
    const static KLocalizedString GHz = ki18nc("Gigahertz unit symbol", "%1 GHz");
    const static KLocalizedString THz = ki18nc("Terahertz unit symbol", "%1 THz");
    const static KLocalizedString PHz = ki18nc("Petahertz unit symbol", "%1 PHz");

    const static KLocalizedString percent = ki18nc("Percent unit", "%1%");
    const static KLocalizedString RPM = ki18nc("Revolutions per minute unit symbol", "%1 RPM");
    const static KLocalizedString C = ki18nc("Celsius unit symbol", "%1°C");
    const static KLocalizedString dBm = ki18nc("Decibels unit symbol", "%1 dBm");
    const static KLocalizedString s = ki18nc("Seconds unit symbol", "%1s");
    const static KLocalizedString V = ki18nc("Volts unit symbol", "%1 V");
    const static KLocalizedString W = ki18nc("Watts unit symbol", "%1 W");
    const static KLocalizedString Wh = ki18nc("Watt-hours unit symbol", "%1 Wh");
    const static KLocalizedString rate = ki18nc("Rate unit symbol", "%1 s⁻¹");
    const static KLocalizedString A = ki18nc("Ampere unit symbol", "%1 A");
    const static KLocalizedString unitless = ki18nc("Unitless", "%1");

    switch (unit) {
    case UnitByte:
        return B;
    case UnitKiloByte:
        return KiB;
    case UnitMegaByte:
        return MiB;
    case UnitGigaByte:
        return GiB;
    case UnitTeraByte:
        return TiB;
    case UnitPetaByte:
        return PiB;

    case UnitByteRate:
        return bps;
    case UnitKiloByteRate:
        return Kbps;
    case UnitMegaByteRate:
        return Mbps;
    case UnitGigaByteRate:
        return Gbps;
    case UnitTeraByteRate:
        return Tbps;
    case UnitPetaByteRate:
        return Pbps;

    case UnitHertz:
        return Hz;
    case UnitKiloHertz:
        return kHz;
    case UnitMegaHertz:
        return MHz;
    case UnitGigaHertz:
        return GHz;
    case UnitTeraHertz:
        return THz;
    case UnitPetaHertz:
        return PHz;

    case UnitCelsius:
        return C;
    case UnitDecibelMilliWatts:
        return dBm;
    case UnitPercent:
        return percent;
    case UnitRate:
        return rate;
    case UnitRpm:
        return RPM;
    case UnitSecond:
        return s;
    case UnitVolt:
        return V;
    case UnitWatt:
        return W;
    case UnitWattHour:
        return Wh;
    case UnitAmpere:
        return A;

    default:
        return unitless;
    }
}

static int unitOrder(Unit unit)
{
    switch (unit) {
    case UnitByte:
    case UnitKiloByte:
    case UnitMegaByte:
    case UnitGigaByte:
    case UnitTeraByte:
    case UnitPetaByte:
    case UnitByteRate:
    case UnitKiloByteRate:
    case UnitMegaByteRate:
    case UnitGigaByteRate:
    case UnitTeraByteRate:
    case UnitPetaByteRate:
        return 1024;

    case UnitHertz:
    case UnitKiloHertz:
    case UnitMegaHertz:
    case UnitGigaHertz:
    case UnitTeraHertz:
    case UnitPetaHertz:
    case UnitWatt:
    case UnitWattHour:
    case UnitAmpere:
        return 1000;

    default:
        return 0;
    }
}

static Unit unitBase(Unit unit)
{
    switch (unit) {
    case UnitByte:
    case UnitKiloByte:
    case UnitMegaByte:
    case UnitGigaByte:
    case UnitTeraByte:
    case UnitPetaByte:
        return UnitByte;

    case UnitByteRate:
    case UnitKiloByteRate:
    case UnitMegaByteRate:
    case UnitGigaByteRate:
    case UnitTeraByteRate:
    case UnitPetaByteRate:
        return UnitByteRate;

    case UnitHertz:
    case UnitKiloHertz:
    case UnitMegaHertz:
    case UnitGigaHertz:
    case UnitTeraHertz:
    case UnitPetaHertz:
        return UnitHertz;

    default:
        return unit;
    }
}

static Unit adjustedUnit(qreal value, Unit unit, MetricPrefix prefix)
{
    const int order = unitOrder(unit);
    if (!order) {
        return unit;
    }

    const Unit baseUnit = unitBase(unit);
    const MetricPrefix basePrefix = MetricPrefix(unit - baseUnit);

    if (prefix == MetricPrefixAutoAdjust) {
        const qreal absoluteValue = value * std::pow(order, int(basePrefix));
        if (absoluteValue > 0) {
            const int targetPrefix = std::log2(absoluteValue) / std::log2(order);
            if (targetPrefix <= MetricPrefixLast) {
                prefix = MetricPrefix(targetPrefix);
            }
        }
        if (prefix == MetricPrefixAutoAdjust) {
            prefix = basePrefix;
        }
    }

    return Unit(prefix + baseUnit);
}

static QString formatNumber(const QVariant &value, Unit unit, MetricPrefix prefix, FormatOptions options)
{
    qreal amount = value.toDouble();

    if (!options.testFlag(FormatOptionShowNull) && (qFuzzyIsNull(amount) || qIsNaN(amount))) {
        return QString();
    }

    const Unit adjusted = adjustedUnit(amount, unit, prefix);
    if (adjusted != unit) {
        amount /= std::pow(unitOrder(unit), adjusted - unit);
    }

    const int precision = (value.type() != QVariant::Double && adjusted <= unit) ? 0 : 1;
    const QString text = QLocale().toString(amount, 'f', precision);

    return unitFormat(adjusted).subs(text).toString();
}

static QString formatTime(const QVariant &value)
{
    const qlonglong seconds = value.toLongLong();

    const QString minutesString = QString::number(seconds / 60);
    const QString secondsScring = QStringLiteral("%1").arg(seconds % 60, 2, 10, QLatin1Char('0'));

    return minutesString + QLatin1Char(':') + secondsScring;
}

qreal Formatter::scaleDownFactor(const QVariant &value, Unit unit, MetricPrefix targetPrefix)
{
    const Unit adjusted = adjustedUnit(value.toDouble(), unit, targetPrefix);
    if (adjusted == unit) {
        return 1;
    }

    return std::pow(unitOrder(unit), adjusted - unit);
}

KLocalizedString Formatter::localizedString(const QVariant &value, Unit unit, MetricPrefix targetPrefix)
{
    const Unit adjusted = adjustedUnit(value.toDouble(), unit, targetPrefix);
    return unitFormat(adjusted);
}

QString Formatter::formatValue(const QVariant &value, Unit unit, MetricPrefix targetPrefix, FormatOptions options)
{
    switch (unit) {
    case UnitByte:
    case UnitKiloByte:
    case UnitMegaByte:
    case UnitGigaByte:
    case UnitTeraByte:
    case UnitPetaByte:
    case UnitByteRate:
    case UnitKiloByteRate:
    case UnitMegaByteRate:
    case UnitGigaByteRate:
    case UnitTeraByteRate:
    case UnitPetaByteRate:
    case UnitHertz:
    case UnitKiloHertz:
    case UnitMegaHertz:
    case UnitGigaHertz:
    case UnitTeraHertz:
    case UnitPetaHertz:
    case UnitPercent:
    case UnitRate:
    case UnitRpm:
    case UnitCelsius:
    case UnitDecibelMilliWatts:
    case UnitVolt:
    case UnitWatt:
    case UnitSecond:
    case UnitAmpere:
        return formatNumber(value, unit, targetPrefix, options);

    case UnitBootTimestamp:
        qCWarning(FORMATTER) << "UnitBootTimestamp is deprecated and is not formatted anymore";
        return value.toString();
    case UnitTime:
        return formatTime(value);

    default:
        return value.toString();
    }
}

QString Formatter::symbol(Unit unit)
{
    // TODO: Is it possible to avoid duplication of these symbols?
    switch (unit) {
    case UnitByte:
        return i18nc("Bytes unit symbol", "B");
    case UnitKiloByte:
        return i18nc("Kilobytes unit symbol", "KiB");
    case UnitMegaByte:
        return i18nc("Megabytes unit symbol", "MiB");
    case UnitGigaByte:
        return i18nc("Gigabytes unit symbol", "GiB");
    case UnitTeraByte:
        return i18nc("Terabytes unit symbol", "TiB");
    case UnitPetaByte:
        return i18nc("Petabytes unit symbol", "PiB");

    case UnitByteRate:
        return i18nc("Bytes per second unit symbol", "B/s");
    case UnitKiloByteRate:
        return i18nc("Kilobytes per second unit symbol", "KiB/s");
    case UnitMegaByteRate:
        return i18nc("Megabytes per second unit symbol", "MiB/s");
    case UnitGigaByteRate:
        return i18nc("Gigabytes per second unit symbol", "GiB/s");
    case UnitTeraByteRate:
        return i18nc("Gigabytes per second unit symbol", "TiB/s");
    case UnitPetaByteRate:
        return i18nc("Gigabytes per second unit symbol", "PiB/s");

    case UnitHertz:
        return i18nc("Hertz unit symbol", "Hz");
    case UnitKiloHertz:
        return i18nc("Kilohertz unit symbol", "kHz");
    case UnitMegaHertz:
        return i18nc("Megahertz unit symbol", "MHz");
    case UnitGigaHertz:
        return i18nc("Gigahertz unit symbol", "GHz");
    case UnitTeraHertz:
        return i18nc("Terahertz unit symbol", "THz");
    case UnitPetaHertz:
        return i18nc("Petahertz unit symbol", "PHz");

    case UnitPercent:
        return i18nc("Percent unit", "%");
    case UnitRpm:
        return i18nc("Revolutions per minute unit symbol", "RPM");
    case UnitCelsius:
        return i18nc("Celsius unit symbol", "°C");
    case UnitDecibelMilliWatts:
        return i18nc("Decibels unit symbol", "dBm");
    case UnitSecond:
        return i18nc("Seconds unit symbol", "s");
    case UnitVolt:
        return i18nc("Volts unit symbol", "V");
    case UnitWatt:
        return i18nc("Watts unit symbol", "W");
    case UnitWattHour:
        return i18nc("Watt-hours unit symbol", "Wh");
    case UnitRate:
        return i18nc("Rate unit symbol", "s⁻¹");
    case UnitAmpere:
        return i18nc("Ampere unit symbol", "A");

    default:
        return QString();
    }
}

qreal Formatter::maximumLength(Unit unit, const QFont &font)
{
    auto order = unitOrder(unit);

    QString maximum;
    switch (unitBase(unit)) {
    case UnitByte:
        maximum = formatValue(order - 0.5, UnitMegaByte, MetricPrefixMega);
        break;
    case UnitByteRate:
        maximum = formatValue(order - 0.5, UnitMegaByteRate, MetricPrefixMega);
        break;
    case UnitHertz:
        maximum = formatValue(order - 0.5, UnitMegaHertz, MetricPrefixMega);
        break;
    case UnitPercent:
        maximum = formatValue(9999.9, UnitPercent);
        break;
    default:
        return -1.0;
    }

    auto metrics = QFontMetrics{font};
    return metrics.horizontalAdvance(maximum);
}

} // namespace KSysGuard
