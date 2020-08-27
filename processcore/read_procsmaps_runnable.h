/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QObject>
#include <QRunnable>

namespace KSysGuard {

class ReadProcSmapsRunnable : public QObject, public QRunnable
{
    Q_OBJECT
public:
    ReadProcSmapsRunnable(const QString &dir);

    void run() override;

    qlonglong pss() const;

    Q_SIGNAL void finished();

private:
    QString m_dir;
    qlonglong m_pss = 0;
};

} // namespace KSysGuard
