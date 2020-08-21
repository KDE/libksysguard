/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "Task.h"

#include <QFile>

#include <thread>
#include <fstream>

using namespace KSysGuard;

Task::Task(const QString& filePath)
    : QObject(nullptr)
    , m_filePath(filePath)
    , m_finished(false)
    , m_success(true)
{
    setAutoDelete(false);
}

bool Task::isFinished() const
{
    return m_finished;
}

bool Task::isSuccessful() const
{
    return m_success;
}

void Task::setSuccess(bool success)
{
    m_success.store(success);
}

void Task::setFinished(bool success)
{
    m_finished.store(success);
}

void Task::run()
{
    Q_EMIT started();

    std::ifstream stream{m_filePath.toUtf8().data()};
    if (!stream.is_open()) {
        m_finished.store(true);
        m_success.store(false);
        Q_EMIT finished();
        return;
    }

    QByteArray buffer{1024, '\0'};
    while (stream.getline(buffer.data(), buffer.size())) {
        if (!processLine(buffer.left(stream.gcount()))) {
            break;
        }
    }

    m_finished.store(true);
    Q_EMIT finished();
}
