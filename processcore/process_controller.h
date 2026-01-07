/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KSYSGUARD_PROCESSCONTROLLER_H
#define KSYSGUARD_PROCESSCONTROLLER_H

#include <memory>

#include <signal.h>

#include <QObject>
#include <QVariant>
#include <qqmlintegration.h>

#include "enums.h"
#include "process.h"

#include "processcore_export.h"

class QWindow;

/**
 * Control processes' priority, scheduling and sending signals.
 *
 * This class contains methods for sending signals to processes, setting their
 * priority and setting their scheduler. It will first try to manipulate the
 * processes directly, if that fails, it will use KAuth to try and perform the
 * action as root.
 */
namespace KSysGuard
{
class PROCESSCORE_EXPORT ProcessController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    ProcessController(QObject *parent = nullptr);
    ~ProcessController() override;

    using Signal = Enums::Signal::Signal;
    using Result = Enums::Result::Result;
    using Scheduler = Enums::Scheduler::Scheduler;
    using IoPriority = Enums::IoPriority::IoPriority;

    /**
     * The window used as parent for any dialogs that get shown.
     */
    QWindow *window() const;
    /**
     * Set the window to use as parent for dialogs.
     *
     * \param window The window to use.
     */
    void setWindow(QWindow *window);

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
    Q_INVOKABLE Result sendSignal(const QList<int> &pids, int signal);
    /**
     * \overload Result sendSignal(const QList<int> &pids, int signal)
     */
    Q_INVOKABLE Result sendSignal(const QList<long long> &pids, int signal);
    /**
     * \overload Result sendSignal(const QList<int> &pids, int signal)
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
    Q_INVOKABLE Result setPriority(const QList<int> &pids, int priority);
    /**
     * \overload Result setPriority(const QList<int> &pids, int priority)
     */
    Q_INVOKABLE Result setPriority(const QList<long long> &pids, int priority);
    /**
     * \overload Result setPriority(const QList<int> &pids, int priority)
     */
    Q_INVOKABLE Result setPriority(const QVariantList &pids, int priority);

    /**
     * Get the priority (niceness) of a process.
     *
     * If the retrieval fails, it still returns the default value.
     *
     * \param pid The pid to get the priority from.
     *
     * \return The priority of the process.
     */
    Q_INVOKABLE int priority(long long pid);

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
    Q_INVOKABLE Result setCpuScheduler(const QList<int> &pids, Scheduler scheduler, int priority);
    /**
     * \overload Result setCpuScheduler(const QList<int> &pids, Scheduler scheduler, int priority)
     */
    Q_INVOKABLE Result setCpuScheduler(const QList<long long> &pids, Scheduler scheduler, int priority);
    /**
     * \overload Result setCpuScheduler(const QList<int> &pids, Scheduler scheduler, int priority)
     */
    Q_INVOKABLE Result setCpuScheduler(const QVariantList &pids, Scheduler scheduler, int priority);

    /**
     * Get the CPU scheduling policy and priority of a process.
     *
     * If the retrieval fails, it still returns the default value.
     *
     * \param pid The pid to get the scheduler from.
     *
     * \return The scheduler of the process.
     */
    Q_INVOKABLE Scheduler cpuScheduler(long long pid);

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
    Q_INVOKABLE Result setIoScheduler(const QList<int> &pids, IoPriority priorityClass, int priority);
    /**
     * \overload Result setIoScheduler(const QList<int> &pids, IoPriority priorityClass, int priority)
     */
    Q_INVOKABLE Result setIoScheduler(const QList<long long> &pids, IoPriority priorityClass, int priority);
    /**
     * \overload Result setIoScheduler(const QList<int> &pids, IoPriority priorityClass, int priority)
     */
    Q_INVOKABLE Result setIoScheduler(const QVariantList &pids, IoPriority priorityClass, int priority);

    /**
     * Get the IO priority of a process.
     *
     * If the retrieval fails, it still returns the default value.
     *
     * \param pid The pid to get the IO priority from.
     *
     * \return The IO priority of the process.
     */
    Q_INVOKABLE int ioPriority(long long pid);

    /**
     * Get the IO scheduling policy of a process.
     *
     * If the retrieval fails, it still returns the default value.
     *
     * \param pid The pid to get the IO scheduler from.
     *
     * \return The IO scheduler of the process.
     */
    Q_INVOKABLE IoPriority ioScheduler(long long pid);

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
