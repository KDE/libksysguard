/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GUITESTPROCESS_H
#define GUITESTPROCESS_H

#include <QObject>
class testGuiProcess : public QObject
{
    Q_OBJECT
private slots:
    void testGUI();
};

#endif
