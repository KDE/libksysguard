/*
 * Copyright 2020  Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QTest>
#include <QLocale>

#include "Formatter.h"
#include "Unit.h"

#include <unistd.h>
#ifdef Q_OS_OSX
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#define QSL QStringLiteral

class FormatterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        // Ensure we use a known locale for the test.
        QLocale::setDefault(QLocale{QLocale::English, QLocale::UnitedStates});
    }

    void testDouble_data()
    {
        QTest::addColumn<double>("input");
        QTest::addColumn<KSysGuard::Unit>("unit");
        QTest::addColumn<QString>("output");

        QTest::newRow("1.0, B") << 1.0 << KSysGuard::UnitByte << QSL("1.0 B");
        QTest::newRow("1.0, KiB") << 1.0 << KSysGuard::UnitKiloByte << QSL("1.0 KiB");
        QTest::newRow("1.0, KiB/s") << 1.0 << KSysGuard::UnitKiloByteRate << QSL("1.0 KiB/s");
        QTest::newRow("1.0, %") << 1.0 << KSysGuard::UnitPercent << QSL("1.0%");

        QTest::newRow("0.213, B") << 0.213 << KSysGuard::UnitByte << QString::number(0.2) + QSL(" B");
        QTest::newRow("5.647, KiB") << 5.647 << KSysGuard::UnitKiloByte << QString::number(5.6) + QSL(" KiB");
        QTest::newRow("99.99, KiB/s") << 99.99 << KSysGuard::UnitKiloByteRate << QString::number(100.0, 'f', 1) + QSL(" KiB/s");
        QTest::newRow("0.2567, %") << 0.2567 << KSysGuard::UnitPercent << QString::number(0.3) + QSL("%");
    }

    void testDouble()
    {
        QFETCH(double, input);
        QFETCH(KSysGuard::Unit, unit);
        QFETCH(QString, output);

        auto formatted = KSysGuard::Formatter::formatValue(input, unit);
        QCOMPARE(formatted, output);
    }

    void testBootTimeStampAgo_data()
    {
        QTest::addColumn<int>("inputms");
        QTest::addColumn<QString>("outputago");
        QTest::newRow("1 ms") <<  1 << QSL("0s ago");
        QTest::newRow("1 s") << 1000 << QSL("1s ago");
        QTest::newRow("10 s") << 10 * 1000 << QSL("10s ago");
        QTest::newRow("1 s 500ms") << 1000 + 500 << QSL("1s ago");
        QTest::newRow("1 m") << 60 * 1000 << QSL("1m 0s ago");
        QTest::newRow("1 m 1s") << 60 * 1000 + 1000 << QSL("1m 1s ago");
        QTest::newRow("1h") << (60 * 60 * 1000) << QSL("1h 0m 0s ago");
        QTest::newRow("1h 1 m 1s") << (60 * 60 * 1000) + 60  * 1000 + 1000 << QSL("1h 1m 1s ago");
        QTest::newRow("24h") << (60 * 60 * 1000) * 24  << QSL("1 day 0h 0m ago");
        QTest::newRow("25h 1 m") << (60 * 60 * 1000) * 25 + 60  * 1000 + 1000 << QSL("1 day 1h 1m ago");
        QTest::newRow("25h 1 m 1s") << (60 * 60 * 1000) * 25 + 60  * 1000 + 1000 << QSL("1 day 1h 1m ago");
        QTest::newRow("48h") << (60 * 60 * 1000) * 48 << QSL("2 days 0h 0m ago");
    }

    void testBootTimeStampAgo()
    {
        QFETCH(int, inputms);
        QFETCH(QString, outputago);
        const long clockTicksPerSecond = sysconf(_SC_CLK_TCK);
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
        long ticks = clockTicksPerSecond * (tp.tv_sec - inputms / 1000);
        auto formatted = KSysGuard::Formatter::formatValue(QVariant::fromValue<long>(ticks),
            KSysGuard::UnitBootTimestamp);
        QCOMPARE(formatted, QDateTime::currentDateTime().addMSecs(-inputms).toString(Qt::DefaultLocaleLongDate));
        formatted = KSysGuard::Formatter::formatValue(QVariant::fromValue<long>(ticks),
            KSysGuard::UnitBootTimestamp, KSysGuard::MetricPrefixAutoAdjust, KSysGuard::FormatOptionAgo);
        QCOMPARE(formatted, outputago);
    }

    void testFormatTime_data()
    {
        QTest::addColumn<int>("input");
        QTest::addColumn<QString>("output");
        QTest::newRow("1 s") << 1 << QSL("0:01");
        QTest::newRow("10 s") << 10 << QSL("0:10");
        QTest::newRow("1 m") << 60 << QSL("1:00");
        QTest::newRow("10m") << 60 * 10 << QSL("10:00");
        QTest::newRow("1h") << 60 * 60 << QSL("60:00");
        QTest::newRow("1h 1 m 1s") << (60 * 60) + 60 + 1 << QSL("61:01");
    }

    void testFormatTime()
    {
        QFETCH(int, input);
        QFETCH(QString, output);
        auto formatted =  KSysGuard::Formatter::formatValue(input, KSysGuard::UnitTime);
        QCOMPARE(formatted, output);
    }

};

QTEST_MAIN(FormatterTest);

#include "formattertest.moc"
