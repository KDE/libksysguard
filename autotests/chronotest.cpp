/*
    SPDX-FileCopyrightText: 2014 Gregor Mi <codestruct@posteo.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QDebug>
#include <QtCore>
#include <QtTest>

#include "chronotest.h"
#include <processui/timeutil.h>

void testChrono::testTimeMethods()
{
    qDebug() << "TimeUtil::systemUptimeSeconds()" << TimeUtil::systemUptimeSeconds();
    qDebug() << "TimeUtil::systemUptimeAbsolute()" << TimeUtil::systemUptimeAbsolute();
}

void testChrono::testsecondsToHumanString1()
{
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(0), QStringLiteral("0s ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(1), QStringLiteral("1s ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(59), QStringLiteral("59s ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(1 * 60), QStringLiteral("1m 0s ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(1 * 60 + 1), QStringLiteral("1m 1s ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(1 * 60 + 59), QStringLiteral("1m 59s ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(2 * 60), QStringLiteral("2m 0s ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(59 * 60 + 59), QStringLiteral("59m 59s ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(1 * 60 * 60), QStringLiteral("1h 0m 0s ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(1 * 60 * 60 + 59 * 60 + 59), QStringLiteral("1h 59m 59s ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(2 * 60 * 60), QStringLiteral("2h 0m 0s ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(23 * 60 * 60 + 59 * 60 + 59), QStringLiteral("23h 59m 59s ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(24 * 60 * 60), QStringLiteral("1 day 0h 0m ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(25 * 60 * 60 + 59 * 60), QStringLiteral("1 day 1h 59m ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(25 * 60 * 60 + 59 * 60 + 59), QStringLiteral("1 day 1h 59m ago")); // seconds are omitted now
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(2 * 24 * 60 * 60), QStringLiteral("2 days 0h 0m ago"));
    QCOMPARE(TimeUtil::secondsToHumanElapsedString(10 * 24 * 60 * 60), QStringLiteral("10 days 0h 0m ago"));
}

QTEST_MAIN(testChrono)
