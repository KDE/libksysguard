/*
    KSysGuard, the KDE System Guard

    SPDX-FileCopyrightText: 2008 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

*/

#ifndef _LsofSearchWidget_h_
#define _LsofSearchWidget_h_

#include <QDialog>

class Ui_LsofSearchWidget;

/**
 * This class creates and handles a simple dialog to change the scheduling
 * priority of a process.
 */
class LsofSearchWidget : public QDialog
{
    Q_OBJECT

public:
    explicit LsofSearchWidget(QWidget *parent);
    ~LsofSearchWidget();

private:
    Ui_LsofSearchWidget *ui;
};

#endif
