/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QRunnable>
#include <QString>

#include "process.h"

class QFile;

namespace KSysGuard {

class Task : public QObject, public QRunnable
{
    Q_OBJECT
public:
    Task(const QString &filePath);

    virtual void run() override;

    bool isFinished() const;
    bool isSuccessful() const;

    virtual void updateProcess(Process* process) = 0;

    Q_SIGNAL void started();
    Q_SIGNAL void finished();

protected:
    void setSuccess(bool success);
    void setFinished(bool finished);

    virtual bool processLine(const QByteArray &data) = 0;

private:
    QString m_filePath;
    std::atomic_bool m_finished;
    std::atomic_bool m_success;
};

} // namespace KSysGuard

#endif // TASK_H
