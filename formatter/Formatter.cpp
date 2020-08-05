/*
    Copyright (C) 2019 Vlad Zahorodnii <vladzzag@gmail.com>
    Copyright (C) 2020 David Redondo <kdeq@david-redondo.de>

    formatBootTimestamp is based on TimeUtil class:
    Copyright (C) 2014 Gregor Mi <codestruct@posteo.org>

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

#include "Formatter.h"

#include <KLocalizedString>

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
#include <KFormat>

namespace KSysGuard
{

// TODO: Is there a bit nicer way to handle formatting?


static KFormat::Unit kformatUnit(Unit unit) {
    switch(unit) {
    case UnitByte:
    case UnitKiloByte:
    case UnitMegaByte:
    case UnitGigaByte:
    case UnitTeraByte:
    case UnitPetaByte:
        return KFormat::Unit::Byte;
    case UnitHertz:
    case UnitKiloHertz:
    case UnitMegaHertz:
    case UnitGigaHertz:
    case UnitTeraHertz:
    case UnitPetaHertz:
        return KFormat::Unit::Hertz;
    default:
        return KFormat::Unit::Other;
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

static KFormat::UnitPrefix kFormatPrefix(MetricPrefix prefix) {
    switch (prefix) {
    case MetricPrefixAutoAdjust:
        return KFormat::UnitPrefix::AutoAdjust;
    case MetricPrefixUnity:
        return KFormat::UnitPrefix::Unity;
    case MetricPrefixKilo:
         return KFormat::UnitPrefix::Kilo;
    case MetricPrefixMega:
         return KFormat::UnitPrefix::Mega;
    case MetricPrefixGiga:
        return KFormat::UnitPrefix::Giga;
    case MetricPrefixTera:
          return KFormat::UnitPrefix::Tera;
    case MetricPrefixPeta:
          return KFormat::UnitPrefix::Exa;
    }
    return KFormat::UnitPrefix::Unity;
}

static QString formatNumber(const QVariant &value, Unit unit, MetricPrefix prefix, FormatOptions options)
{
    qreal amount = value.toDouble();

    if (!options.testFlag(FormatOptionShowNull) && qFuzzyIsNull(amount)) {
        return QString();
    }
    int order = unitOrder(unit);

    if (!order) {
        prefix = MetricPrefixUnity;
    }

    const int precision = (value.type() != QVariant::Double) ? 0 : 1;
    KFormat::Unit kfUnit = kformatUnit(unit);
    KFormat::BinaryUnitDialect dialect = order == 1024 ? KFormat::IECBinaryDialect : KFormat::MetricBinaryDialect;
    KFormat::UnitPrefix kFToPrefix = kFormatPrefix(prefix);
    KFormat::UnitPrefix  kFFromPrefix = kFormatPrefix(MetricPrefix(unit - unitBase(unit)));

    KFormat format;
    if (kfUnit != KFormat::Unit::Other) {
        return format.formatValue(amount, kfUnit, precision, kFFromPrefix, kFToPrefix, dialect);
    } else {
        return format.formatValue(amount, Formatter::symbol(unitBase(unit)), precision, kFFromPrefix, kFToPrefix, dialect);
    }
}

static QString formatTime(const QVariant &value)
{

    KFormat format;
    return format.formatDuration(value.toLongLong() * 1000, KFormat::DurationFormatOption::FoldHours);
}

static QString formatBootTimestamp(const QVariant &value, FormatOptions options)
{
    const qlonglong clockTicksSinceSystemBoot = value.toLongLong();
    const QDateTime now = QDateTime::currentDateTime();

#ifdef Q_OS_OSX
    clock_serv_t cclock;
    mach_timespec_t tp;

    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
    clock_get_time(cclock, &tp);
    mach_port_deallocate(mach_task_self(), cclock);
#else
    timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);
#endif
    const QDateTime systemBootTime = now.addSecs(-tp.tv_sec);

    const long clockTicksPerSecond = sysconf(_SC_CLK_TCK);
    const qreal secondsSinceSystemBoot = qreal(clockTicksSinceSystemBoot) / clockTicksPerSecond;
    const QDateTime absoluteStartTime = systemBootTime.addSecs(secondsSinceSystemBoot);

    if (!options.testFlag(FormatOptionAgo)) {
         // Maybe KFormat::formatRelativeDateTime to get "Yesterday, Today"?
        return QLocale().toString(absoluteStartTime);
    }

    KFormat format;
    return i18nc("%1 is a time duration formatted according to KFormat::formatSpelloutDuration",
        "%1 ago", format.formatDuration(absoluteStartTime.msecsTo(now), KFormat::InitialDuration));
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
        return formatNumber(value, unit, targetPrefix, options);

    case UnitBootTimestamp:
        return formatBootTimestamp(value, options);
    case UnitTime:
        return formatTime(value);

    default:
        return value.toString();
    }
}

// Maybe move that to unit.h?
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
    case UnitRate:
        return i18nc("Rate unit symbol", "s⁻¹");

    default:
        return QString();
    }
}

} // namespace KSysGuard
