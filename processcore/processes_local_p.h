/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PROCESSES_LOCAL_H_
#define PROCESSES_LOCAL_H_

#include <unistd.h> //For sysconf

#include <QObject>
#include <QSet>

#include "processes.h"

namespace KSysGuard
{
class Process;

/**
 * This is the OS specific code to get process information for the local host.
 *
 * To port this to other operating systems you need to make a processes_(osname).cpp  file
 * which implements all of the function below.  If you need private functions/variables etc put them in
 * the Private class.
 *
 * @author John Tapsell <tapsell@kde.org>
 */
class ProcessesLocal : public QObject
{
    Q_OBJECT
public:
    ProcessesLocal();
    ~ProcessesLocal();

    /** \brief Get a set of the currently running process PIDs.
     *
     *  To get information about processes, this will be the first function called.
     */
    QSet<long> getAllPids();

    /** \brief Return the parent PID for the given process PID.
     *
     *  For each of the PIDs that getAllPids() returns, getParentPid will be called.
     *  This is used to setup the tree structure.
     *  For a particular PID, this is guaranteed to be called before updateProcessInfo for that PID.
     *  However this may be called several times in a row before the updateProcessInfo is called, so be careful
     *  if you want to try to preserve state in Private.
     */
    long getParentPid(long pid);

    /** \brief Fill in the given Process class with information for given PID.
     *
     *  This will be called for every PID, after getParentPid() has been called for the same parameter.
     *
     *  The process->pid() process->ppid and process->parent  are all guaranteed
     *  to be filled in correctly and process->parent will be non null.
     */
    bool updateProcessInfo(long pid, Process *process);

    /** \brief Send the specified named POSIX signal to the process given.
     *
     *  For example, to indicate for process 324 to STOP do:
     *  \code
     *    #include <signals.h>
     *     ...
     *
     *    KSysGuard::Processes::sendSignal(324, SIGSTOP);
     *  \endcode
     *  @return Error::NoError if successful
     *
     */
    Processes::Error sendSignal(long pid, int sig);

    /** \brief Set the scheduler for a process.
     *
     * This is defined according to POSIX.1-2001
     *  See "man sched_setscheduler" for more information.
     *
     *  @p priorityClass One of SCHED_FIFO, SCHED_RR, SCHED_OTHER, and SCHED_BATCH
     *  @p priority Set to 0 for SCHED_OTHER and SCHED_BATCH.  Between 1 and 99 for SCHED_FIFO and SCHED_RR
     *  @return Error::NoError if successful
     */

    /** \brief Return the total amount of physical memory in KiB.
     *
     *  This is fast (just a system call in most OSes)
     *  Returns 0 on error
     */
    long long totalPhysicalMemory();

    /**
     * The total amount of swap memory that is available on the system.
     */
    long long totalSwapMemory();

    /** \brief Return the number of processor cores enabled.
     *
     *  (A system can disable processors.  Disabled processors are not counted here).
     *  This is fast (just a system call on most OSes) */
    long numberProcessorCores()
#ifdef _SC_NPROCESSORS_ONLN
    {
        return sysconf(_SC_NPROCESSORS_ONLN);
    } // Should work on any recent posix system
#else
        ;
#endif

    /** \brief Update the process information for all processes.
     *
     *  Get all the current process information from the machine.  When done, emit updateAllProcesses().
     */
    void updateAllProcesses(Processes::UpdateFlags updateFlags)
    {
        mUpdateFlags = updateFlags;
        emit processesUpdated();
    } // For local machine, there is no delay

Q_SIGNALS:
    /** \brief This is emitted when the processes have been updated, and the view should be refreshed.
     */
    void processesUpdated();

    void processUpdated(long pid, const KSysGuard::Process::Updates &changes);

private:
    /**
     * You can use this for whatever data you want.
     * Be careful about preserving state in between getParentPid and updateProcessInfo calls
     * if you chose to do that. getParentPid may be called several times
     * for different pids before the relevant updateProcessInfo calls are made.
     * This is because the tree structure has to be sorted out first.
     */
    class Private;
    Private *d;
    Processes::UpdateFlags mUpdateFlags;
};
}
#endif
