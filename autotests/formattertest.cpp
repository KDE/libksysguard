/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QLocale>
#include <QTest>

#include "Formatter.h"
#include "Unit.h"

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
        auto formatted = KSysGuard::Formatter::formatValue(input, KSysGuard::UnitTime);
        QCOMPARE(formatted, output);
    }
};

QTEST_MAIN(FormatterTest);

#include "formattertest.moc"
