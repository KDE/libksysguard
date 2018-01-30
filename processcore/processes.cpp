/*  This file is part of the KDE project

    Copyright (C) 2007 John Tapsell <tapsell@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "processes.h"
#include "processes_base_p.h"
#include "processes_local_p.h"
#include "processes_remote_p.h"
#include "processes_atop_p.h"
#include "process.h"

#include <QHash>
#include <QSet>
#include <QMutableSetIterator>
#include <QByteArray>

//for sysconf
#include <unistd.h>

/* if porting to an OS without signal.h  please #define SIGTERM to something */
#include <signal.h>


namespace KSysGuard
{
  class Processes::Private
  {
    public:
      Private(Processes *q_ptr) {
        mFakeProcess.setParent(&mFakeProcess);
        mAbstractProcesses = nullptr;
        mHistoricProcesses = nullptr;
        mIsLocalHost = true;
        mProcesses.insert(-1, &mFakeProcess);
        mElapsedTimeMilliSeconds = 0;
        mHavePreviousIoValues = false;
        mUpdateFlags = nullptr;
        mUsingHistoricalData = false;
        q = q_ptr;
    }
      ~Private();
      void markProcessesAsEnded(long pid);

      QSet<long> mToBeProcessed;
      QSet<long> mProcessedLastTime;
      QSet<long> mEndedProcesses; ///< Processes that have finished

      QHash<long, Process *> mProcesses; ///< This must include mFakeProcess at pid -1
      QList<Process *> mListProcesses;   ///< A list of the processes.  Does not include mFakeProcesses
      Process mFakeProcess; ///< A fake process with pid -1 just so that even init points to a parent

      AbstractProcesses *mAbstractProcesses; ///< The OS specific code to get the process information
      ProcessesATop *mHistoricProcesses; ///< A way to get historic information about processes
      bool mIsLocalHost; ///< Whether this is localhost or not

      QTime mLastUpdated; ///< This is the time we last updated.  Used to calculate cpu usage.
      long mElapsedTimeMilliSeconds; ///< The number of milliseconds  (1000ths of a second) that passed since the last update

      Processes::UpdateFlags mUpdateFlags;
      bool mHavePreviousIoValues; ///< This is whether we updated the IO value on the last update
      bool mUsingHistoricalData; ///< Whether to return historical data for updateProcess() etc
      Processes *q;
  };

Processes::Private::~Private() {
  Q_FOREACH(Process *process, mProcesses) {
    if(process != &mFakeProcess)
      delete process;
  }
  mProcesses.clear();
  mListProcesses.clear();
  delete mAbstractProcesses;
  mAbstractProcesses = nullptr;
}

Processes::Processes(const QString &host, QObject *parent) : QObject(parent), d(new Private(this))
{
    if(host.isEmpty()) {
        d->mAbstractProcesses = new ProcessesLocal();
    } else {
        ProcessesRemote *remote = new ProcessesRemote(host);
        d->mAbstractProcesses = remote;
        connect(remote, &ProcessesRemote::runCommand, this, &Processes::runCommand);
    }
    d->mIsLocalHost = host.isEmpty();
    connect( d->mAbstractProcesses, &AbstractProcesses::processesUpdated, this, &Processes::processesUpdated);
}
Processes::~Processes()
{
    delete d;
}

Processes::Error Processes::lastError() const
{
    return d->mAbstractProcesses->errorCode;
}
Process *Processes::getProcess(long pid) const
{
    return d->mProcesses.value(pid);
}

const QList<Process *> &Processes::getAllProcesses() const
{
    return d->mListProcesses;
}

int Processes::processCount() const
{
    return d->mListProcesses.count();
}

bool Processes::updateProcess( Process *ps, long ppid)
{
    Process *parent = d->mProcesses.value(ppid, &d->mFakeProcess);
    Q_ASSERT(parent);  //even init has a non-null parent - the mFakeProcess

    if(ps->parent() != parent) {
        emit beginMoveProcess(ps, parent/*new parent*/);
        //Processes has been reparented
        Process *p = ps;
        do {
            p = p->parent();
            p->numChildren()--;
        } while (p->pid()!= -1);
        Q_ASSERT(ps != parent);
        ps->parent()->children().removeAll(ps);
        ps->setParent(parent);  //the parent has changed
        parent->children().append(ps);
        p = ps;
        do {
            p = p->parent();
            p->numChildren()++;
        } while (p->pid()!= -1);
        emit endMoveProcess();
        Q_ASSERT(ps != parent);
        ps->setParent(parent);
    }

    ps->setParentPid(ppid);

    bool success = updateProcessInfo(ps);
    emit processChanged(ps, false);

    return success;
}

bool Processes::updateProcessInfo(Process *ps) {
    //Now we can actually get the process info
    qlonglong oldUserTime = ps->userTime();
    qlonglong oldSysTime = ps->sysTime();

    qlonglong oldIoCharactersRead = 0;
    qlonglong oldIoCharactersWritten = 0;
    qlonglong oldIoReadSyscalls = 0;
    qlonglong oldIoWriteSyscalls = 0;
    qlonglong oldIoCharactersActuallyRead = 0;
    qlonglong oldIoCharactersActuallyWritten = 0;

    if(d->mUpdateFlags.testFlag(Processes::IOStatistics)) {
        oldIoCharactersRead = ps->ioCharactersRead();
        oldIoCharactersWritten = ps->ioCharactersWritten();
        oldIoReadSyscalls = ps->ioReadSyscalls();
        oldIoWriteSyscalls = ps->ioWriteSyscalls();
        oldIoCharactersActuallyRead = ps->ioCharactersActuallyRead();
        oldIoCharactersActuallyWritten = ps->ioCharactersActuallyWritten();
    }

    ps->setChanges(Process::Nothing);
    bool success;
    if(d->mUsingHistoricalData)
        success = d->mHistoricProcesses->updateProcessInfo(ps->pid(), ps);
    else
        success = d->mAbstractProcesses->updateProcessInfo(ps->pid(), ps);

    //Now we have the process info.  Calculate the cpu usage and total cpu usage for itself and all its parents
    if(!d->mUsingHistoricalData && d->mElapsedTimeMilliSeconds != 0) {  //Update the user usage and sys usage
#ifndef Q_OS_NETBSD
        /* The elapsed time is the d->mElapsedTimeMilliSeconds
         * (which is of the order 2 seconds or so) plus a small
         * correction where we get the amount of time elapsed since
         * we start processing. This is because the processing itself
         * can take a non-trivial amount of time.  */
        int elapsedTime = ps->elapsedTimeMilliSeconds();
        ps->setElapsedTimeMilliSeconds(d->mLastUpdated.elapsed());
        elapsedTime = ps->elapsedTimeMilliSeconds() - elapsedTime + d->mElapsedTimeMilliSeconds;
        if(elapsedTime) {
            ps->setUserUsage((int)(((ps->userTime() - oldUserTime)*1000.0) / elapsedTime));
            ps->setSysUsage((int)(((ps->sysTime() - oldSysTime)*1000.0) / elapsedTime));
        }
#endif
        if(d->mUpdateFlags.testFlag(Processes::IOStatistics)) {
            if( d->mHavePreviousIoValues ) {
                ps->setIoCharactersReadRate((ps->ioCharactersRead() - oldIoCharactersRead) * 1000.0 / elapsedTime);
                ps->setIoCharactersWrittenRate((ps->ioCharactersWritten() - oldIoCharactersWritten) * 1000.0 / elapsedTime);
                ps->setIoReadSyscallsRate((ps->ioReadSyscalls() - oldIoReadSyscalls) * 1000.0 / elapsedTime);
                ps->setIoWriteSyscallsRate((ps->ioWriteSyscalls() - oldIoWriteSyscalls) * 1000.0 / elapsedTime);
                ps->setIoCharactersActuallyReadRate((ps->ioCharactersActuallyRead() - oldIoCharactersActuallyRead) * 1000.0 / elapsedTime);
                ps->setIoCharactersActuallyWrittenRate((ps->ioCharactersActuallyWritten() - oldIoCharactersActuallyWritten) * 1000.0 / elapsedTime);
            } else
                d->mHavePreviousIoValues = true;
        } else if(d->mHavePreviousIoValues) {
            d->mHavePreviousIoValues = false;
            ps->setIoCharactersReadRate(0);
            ps->setIoCharactersWrittenRate(0);
            ps->setIoReadSyscallsRate(0);
            ps->setIoWriteSyscallsRate(0);
            ps->setIoCharactersActuallyReadRate(0);
            ps->setIoCharactersActuallyWrittenRate(0);
        }
    }
    if(d->mUsingHistoricalData || d->mElapsedTimeMilliSeconds != 0) {
        ps->setTotalUserUsage(ps->userUsage());
        ps->setTotalSysUsage(ps->sysUsage());
        if(ps->userUsage() != 0 || ps->sysUsage() != 0) {
            Process *p = ps->parent();
            while(p->pid() != -1) {
                p->totalUserUsage() += ps->userUsage();
                p->totalSysUsage() += ps->sysUsage();
                emit processChanged(p, true);
                p = p->parent();
            }
        }
    }

    return success;
}

bool Processes::addProcess(long pid, long ppid)
{
    Process *parent = d->mProcesses.value(ppid);
    if(!parent) {
        //Under race conditions, the parent could have already quit
        //In this case, attach to top leaf
        parent = &d->mFakeProcess;
        Q_ASSERT(parent);  //even init has a non-null parent - the mFakeProcess
    }
    //it's a new process - we need to set it up
    Process *ps = new Process(pid, ppid, parent);

    emit beginAddProcess(ps);

    d->mProcesses.insert(pid, ps);

    ps->setIndex(d->mListProcesses.count());
    d->mListProcesses.append(ps);

    ps->parent()->children().append(ps);
    Process *p = ps;
    do {
        Q_ASSERT(p);
        p = p->parent();
        p->numChildren()++;
    } while (p->pid() != -1);
    ps->setParentPid(ppid);

    //Now we can actually get the process info
    bool success = updateProcessInfo(ps);
    emit endAddProcess();
    return success;

}
bool Processes::updateOrAddProcess( long pid)
{
    long ppid;
    if(d->mUsingHistoricalData)
        ppid = d->mHistoricProcesses->getParentPid(pid);
    else
        ppid = d->mAbstractProcesses->getParentPid(pid);

    if (ppid == pid) //Shouldn't ever happen
        ppid = -1;

    if(d->mToBeProcessed.contains(ppid)) {
        //Make sure that we update the parent before we update this one.  Just makes things a bit easier.
        d->mToBeProcessed.remove(ppid);
        d->mProcessedLastTime.remove(ppid); //It may or may not be here - remove it if it is there
        updateOrAddProcess(ppid);
    }

    Process *ps = d->mProcesses.value(pid);
    if(!ps)
        return addProcess(pid, ppid);
    else
        return updateProcess(ps, ppid);
}

void Processes::updateAllProcesses(long updateDurationMS, Processes::UpdateFlags updateFlags)
{
    d->mUpdateFlags = updateFlags;

    if(d->mUsingHistoricalData || d->mLastUpdated.elapsed() >= updateDurationMS || !d->mLastUpdated.isValid())  {
        d->mElapsedTimeMilliSeconds = d->mLastUpdated.restart();
        if(d->mUsingHistoricalData)
            d->mHistoricProcesses->updateAllProcesses(d->mUpdateFlags);
        else
            d->mAbstractProcesses->updateAllProcesses(d->mUpdateFlags);  //For a local machine, this will directly call Processes::processesUpdated()
    }
}

void Processes::processesUpdated() {
    //First really delete any processes that ended last time
    long pid;
    {
        QSetIterator<long> i(d->mEndedProcesses);
        while( i.hasNext() ) {
            pid = i.next();
            deleteProcess(pid);
        }
    }

    if(d->mUsingHistoricalData)
        d->mToBeProcessed = d->mHistoricProcesses->getAllPids();
    else
        d->mToBeProcessed = d->mAbstractProcesses->getAllPids();


    QSet<long> beingProcessed(d->mToBeProcessed); //keep a copy so that we can replace mProcessedLastTime with this at the end of this function

    {
        QMutableSetIterator<long> i(d->mToBeProcessed);
        while( i.hasNext()) {
            pid = i.next();
            i.remove();
            d->mProcessedLastTime.remove(pid); //It may or may not be here - remove it if it is there
            updateOrAddProcess(pid);  //This adds the process or changes an existing one
            i.toFront(); //we can remove entries from this set elsewhere, so our iterator might be invalid.  Reset it back to the start of the set
        }
    }
    {
        QSetIterator<long> i(d->mProcessedLastTime);
        while( i.hasNext()) {
            //We saw these pids last time, but not this time.  That means we have to mark them for deletion now
            pid = i.next();
            d->markProcessesAsEnded(pid);
        }
        d->mEndedProcesses = d->mProcessedLastTime;
    }

    d->mProcessedLastTime = beingProcessed;  //update the set for next time this function is called
    return;
}

void Processes::Private::markProcessesAsEnded(long pid)
{
    Q_ASSERT(pid >= 0);

    Process *process = mProcesses.value(pid);
    if(!process)
        return;
    process->setStatus(Process::Ended);
    emit q->processChanged(process, false);
}
void Processes::deleteProcess(long pid)
{
    Q_ASSERT(pid >= 0);

    Process *process = d->mProcesses.value(pid);
    if(!process)
        return;
    Q_FOREACH( Process *it, process->children()) {
        d->mProcessedLastTime.remove(it->pid());
        deleteProcess(it->pid());
    }

    emit beginRemoveProcess(process);

    d->mProcesses.remove(pid);
    d->mListProcesses.removeAll(process);
    process->parent()->children().removeAll(process);
    Process *p = process;
    do {
        Q_ASSERT(p);
        p = p->parent();
        p->numChildren()--;
    } while (p->pid() != -1);
#ifndef QT_NO_DEBUG
    int i = 0;
#endif
    Q_FOREACH( Process *it, d->mListProcesses ) {
        if(it->index() > process->index())
            it->setIndex(it->index() - 1);
#ifndef QT_NO_DEBUG
        Q_ASSERT(it->index() == i++);
#endif
    }

    delete process;
    emit endRemoveProcess();
}


bool Processes::killProcess(long pid) {
    return sendSignal(pid, SIGTERM);
}

bool Processes::sendSignal(long pid, int sig) {
    d->mAbstractProcesses->errorCode = Unknown;
    if(d->mUsingHistoricalData) {
        d->mAbstractProcesses->errorCode = NotSupported;
        return false;
    }
    return d->mAbstractProcesses->sendSignal(pid, sig);
}

bool Processes::setNiceness(long pid, int priority) {
    d->mAbstractProcesses->errorCode = Unknown;
    if(d->mUsingHistoricalData) {
        d->mAbstractProcesses->errorCode = NotSupported;
        return false;
    }
    return d->mAbstractProcesses->setNiceness(pid, priority);
}

bool Processes::setScheduler(long pid, KSysGuard::Process::Scheduler priorityClass, int priority) {
    d->mAbstractProcesses->errorCode = Unknown;
    if(d->mUsingHistoricalData) {
        d->mAbstractProcesses->errorCode = NotSupported;
        return false;
    }
    return d->mAbstractProcesses->setScheduler(pid, priorityClass, priority);
}

bool Processes::setIoNiceness(long pid, KSysGuard::Process::IoPriorityClass priorityClass, int priority) {
    d->mAbstractProcesses->errorCode = Unknown;
    if(d->mUsingHistoricalData) {
        d->mAbstractProcesses->errorCode = NotSupported;
        return false;
    }
    return d->mAbstractProcesses->setIoNiceness(pid, priorityClass, priority);
}

bool Processes::supportsIoNiceness() {
    if(d->mUsingHistoricalData)
        return false;
    return d->mAbstractProcesses->supportsIoNiceness();
}

long long Processes::totalPhysicalMemory() {
    return d->mAbstractProcesses->totalPhysicalMemory();
}

long Processes::numberProcessorCores() {
    return d->mAbstractProcesses->numberProcessorCores();
}

void Processes::answerReceived( int id, const QList<QByteArray>& answer ) {
    KSysGuard::ProcessesRemote *processes = qobject_cast<KSysGuard::ProcessesRemote *>(d->mAbstractProcesses);
    if(processes)
        processes->answerReceived(id, answer);
}

QList< QPair<QDateTime,uint> > Processes::historiesAvailable() const
{
    if(!d->mIsLocalHost)
        return QList< QPair<QDateTime,uint> >();
    if(!d->mHistoricProcesses)
        d->mHistoricProcesses = new ProcessesATop();

    return d->mHistoricProcesses->historiesAvailable();
}

void Processes::useCurrentData()
{
    if(d->mUsingHistoricalData) {
        delete d->mHistoricProcesses;
        d->mHistoricProcesses = nullptr;
        connect( d->mAbstractProcesses, &AbstractProcesses::processesUpdated, this, &Processes::processesUpdated);
        d->mUsingHistoricalData = false;
    }
}

bool Processes::setViewingTime(const QDateTime &when)
{
    d->mAbstractProcesses->errorCode = Unknown;
    if(!d->mIsLocalHost) {
        d->mAbstractProcesses->errorCode = NotSupported;
        return false;
    }
    if(!d->mUsingHistoricalData) {
        if(!d->mHistoricProcesses)
            d->mHistoricProcesses = new ProcessesATop();
        disconnect( d->mAbstractProcesses, &AbstractProcesses::processesUpdated, this, &Processes::processesUpdated);
        connect( d->mHistoricProcesses, &AbstractProcesses::processesUpdated, this, &Processes::processesUpdated);
        d->mUsingHistoricalData = true;
    }
    return d->mHistoricProcesses->setViewingTime(when);
}

bool Processes::loadHistoryFile(const QString &filename)
{
    d->mAbstractProcesses->errorCode = Unknown;
    if(!d->mIsLocalHost) {
        d->mAbstractProcesses->errorCode = NotSupported;
        return false;
    }
    if(!d->mHistoricProcesses)
        d->mHistoricProcesses = new ProcessesATop(false);

    return d->mHistoricProcesses->loadHistoryFile(filename);
}

QString Processes::historyFileName() const
{
    if(!d->mIsLocalHost || !d->mHistoricProcesses)
        return {};
    return d->mHistoricProcesses->historyFileName();
}
QDateTime Processes::viewingTime() const
{
    if(!d->mIsLocalHost || !d->mHistoricProcesses)
        return QDateTime();
    return d->mHistoricProcesses->viewingTime();
}

bool Processes::isHistoryAvailable() const
{
    if(!d->mIsLocalHost)
        return false;
    if(!d->mHistoricProcesses)
        d->mHistoricProcesses = new ProcessesATop();

    return d->mHistoricProcesses->isHistoryAvailable();
}

}


