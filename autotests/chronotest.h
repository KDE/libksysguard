/*
    SPDX-FileCopyrightText: 2014 Gregor Mi <codestruct@posteo.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CHRONOTEST_H
#define CHRONOTEST_H

#include <QObject>
class testChrono : public QObject
{
    Q_OBJECT

private slots:
    void testTimeMethods();
    void testsecondsToHumanString1();
};

#endif
