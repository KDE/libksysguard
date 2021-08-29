/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QDebug>
#include <QtCore>
#include <QtTest>
#include <qtest.h>

#include <processui/ksysguardprocesslist.h>

#include "guitest.h"

void testGuiProcess::testGUI()
{
    KSysGuardProcessList processlist(NULL);

    QTime t;
    t.start();

    for (int i = 0; i < 10; i++) {
        processlist.updateList();
    }
    qDebug() << "time taken: " << t.elapsed() << "ms";
}

QTEST_MAIN(testGuiProcess)
