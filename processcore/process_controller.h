/*
 * This file is part of KSysGuard.
 * Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KSYSGUARD_PROCESSCONTROLLER_H
#define KSYSGUARD_PROCESSCONTROLLER_H

#include <memory>

#include <QObject>
#include <QVariant>

#include "process.h"

class QWidget;

/**
 * Control processes' priority, scheduling and sending signals.
 *
 * This class contains methods for sending signals to processes, setting their
 * priority and setting their scheduler. It will first try to manipulate the
 * processes directly, if that fails, it will use KAuth to try and perform the
 * action as root.
 */
namespace KSysGuard {

class Q_DECL_EXPORT ProcessController : public QObject
{
    Q_OBJECT

public:
    ProcessController(QObject* parent = nullptr);
    ~ProcessController();

    /**
     * A signal that can be sent to a process.
     */
    enum Signal {
        StopSignal,
        ContinueSignal,
        HangupSignal,
        InterruptSignal,
        TerminateSignal,
        KillSignal,
        User1Signal,
        User2Signal
    };
    Q_ENUM(Signal)

    /**
     * What kind of result a call to one of the ProcessController methods had.
     */
    enum class Result {
        Unknown, ///< Something happened, we just do not know what.
        Success, ///< Everything went alright.
        InsufficientPermissions, ///< Some processes require privileges to modify and we failed getting those.
        NoSuchProcess, ///< Tried to modify a process that no longer exists.
        Unsupported, ///< The specified action is not supported.
        UserCancelled, ///< The user cancelled the action, usually when requesting privileges.
        Error, ///< An error occurred when requesting privileges.
    };
    Q_ENUM(Result)

    /**
     * The widget used as parent for any dialogs that get shown.
     */
    QWidget *widget() const;
    /**
     * Set the widget to use as parent for dialogs.
     *
     * \param widget The widget to use.
     */
    void setWidget(QWidget *widget);

    /**
     * Send a signal to a number of processes.
     *
     * This will send \p signal to all processes in \p pids. Should a number of
     * these be owned by different users, an attempt will be made to send the
     * signal as root using KAuth.
     *
     * \param pids A vector of pids to send the signal to.
     * \param signal The signal to send. See Signal for possible values.
     *
     * \return A Result value that indicates whether the action succeeded. Note
     *         that a non-Success result may indicate any of the processes in
     *         \p pids encountered that result.
     */
    Q_INVOKABLE Result sendSignal(const QVector<int> &pids, int signal);
    /**
     * \overload Result sendSignal(const QVector<int> &pids, int signal)
     */
    Q_INVOKABLE Result sendSignal(const QList<long long> &pids, int signal);
    /**
     * \overload Result sendSignal(const QVector<int> &pids, int signal)
     */
    Q_INVOKABLE Result sendSignal(const QVariantList &pids, int signal);

    /**
     * Set the priority (niceness) of a number of processes.
     *
     * This will set the priority of all processes in \p pids to \p priority.
     * Should a number of these be owned by different users, an attempt will be
     * made to send the signal as root using KAuth.
     *
     * \param pids A vector of pids to set the priority of.
     * \param priority The priority to set. Lower means higher priority.
     *
     * \return A Result value that indicates whether the action succeeded. Note
     *         that a non-Success result may indicate any of the processes in
     *         \p pids encountered that result.
     */
    Q_INVOKABLE Result setPriority(const QVector<int> &pids, int priority);
    /**
     * \overload Result setPriority(const QVector<int> &pids, int priority)
     */
    Q_INVOKABLE Result setPriority(const QList<long long> &pids, int priority);
    /**
     * \overload Result setPriority(const QVector<int> &pids, int priority)
     */
    Q_INVOKABLE Result setPriority(const QVariantList &pids, int priority);

    /**
     * Set the CPU scheduling policy and priority of a number of processes.
     *
     * This will set the CPU scheduling policy and priority of all processes in
     * \p pids to \p scheduler and \p priority. Should a number of these be
     * owned by different users, an attempt will be made to send the signal as
     * root using KAuth.
     *
     * \param pids A vector of pids to set the scheduler of.
     * \param scheduler The scheduling policy to use.
     * \param priority The priority to set. Lower means higher priority.
     *
     * \return A Result value that indicates whether the action succeeded. Note
     *         that a non-Success result may indicate any of the processes in
     *         \p pids encountered that result.
     */
    Q_INVOKABLE Result setCPUScheduler(const QVector<int> &pids, Process::Scheduler scheduler, int priority);
    /**
     * \overload Result setCPUScheduler(const QVector<int> &pids, Process::Scheduler scheduler, int priority)
     */
    Q_INVOKABLE Result setCPUScheduler(const QList<long long> &pids, Process::Scheduler scheduler, int priority);
    /**
     * \overload Result setCPUScheduler(const QVector<int> &pids, Process::Scheduler scheduler, int priority)
     */
    Q_INVOKABLE Result setCPUScheduler(const QVariantList &pids, Process::Scheduler scheduler, int priority);

    /**
     * Set the IO scheduling policy and priority of a number of processes.
     *
     * This will set the IO scheduling policy and priority of all processes in
     * \p pids to \p priorityClass and \p priority. Should a number of these be
     * owned by different users, an attempt will be made to send the signal as
     * root using KAuth.
     *
     * \param pids A vector of pids to set the scheduler of.
     * \param priorityClass The scheduling policy to use.
     * \param priority The priority to set. Lower means higher priority.
     *
     * \return A Result value that indicates whether the action succeeded. Note
     *         that a non-Success result may indicate any of the processes in
     *         \p pids encountered that result.
     */
    Q_INVOKABLE Result setIOScheduler(const QVector<int> &pids, Process::IoPriorityClass priorityClass, int priority);
    /**
     * \overload Result setIOScheduler(const QVector<int> &pids, Process::IoPriorityClass priorityClass, int priority)
     */
    Q_INVOKABLE Result setIOScheduler(const QList<long long> &pids, Process::IoPriorityClass priorityClass, int priority);
    /**
     * \overload Result setIOScheduler(const QVector<int> &pids, Process::IoPriorityClass priorityClass, int priority)
     */
    Q_INVOKABLE Result setIOScheduler(const QVariantList &pids, Process::IoPriorityClass priorityClass, int priority);

    /**
     * Convert a Result value to a user-visible string.
     *
     *
     */
    Q_INVOKABLE QString resultToString(Result result);

private:
    class Private;
    const std::unique_ptr<Private> d;
};

}

#endif // KSYSGUARD_PROCESSCONTROLLER_H
