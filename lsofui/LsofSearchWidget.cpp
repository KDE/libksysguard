/*
    KSysGuard, the KDE System Guard

    SPDX-FileCopyrightText: 1999 Chris Schlaeger <cs@kde.org>
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later


*/

#include <KConfigGroup>
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "ui_LsofSearchWidget.h"

LsofSearchWidget::LsofSearchWidget(QWidget *parent, int pid)
    : QDialog(parent)
{
    setObjectName("Renice Dialog");
    setModal(true);
    setWindowTitle(i18n("Renice Process"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainWidget);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QWidget *widget = new QWidget(this);
    mainLayout->addWidget(widget);
    mainLayout->addWidget(buttonBox);
    ui = new Ui_LsofSearchWidget();
    ui->setupUi(widget);
    ui->klsofwidget->setPid(pid);
    ktreewidgetsearchline
}

LsofSearchWidget::~LsofSearchWidget()
{
    delete ui;
}
