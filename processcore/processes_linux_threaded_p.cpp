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

#include "processes_local_p.h"
#include "process.h"

#include <klocalizedstring.h>

#include <QFile>
#include <QHash>
#include <QSet>
#include <QByteArray>
#include <QTextStream>
#include <QThreadPool>
#include <QMutex>
#include <QSemaphore>
#include <QDebug>
#include <QWaitCondition>
#include <QCoreApplication>

#include <iostream>

#include "linux_threaded/Task.h"
#include "linux_threaded/ReadProcStatusTask.h"
#include "linux_threaded/ReadProcCGroupTask.h"
#include "linux_threaded/ReadProcAttrTask.h"
#include "linux_threaded/ReadProcCmdlineTask.h"
#include "linux_threaded/ReadProcSmapsTask.h"
#include "linux_threaded/ReadProcStatmTask.h"
#include "linux_threaded/ReadProcStatTask.h"
#include "linux_threaded/ReadProcIoTask.h"
#include "linux_threaded/GetNicenessTask.h"

//for sysconf
#include <unistd.h>
//for kill and setNice
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/resource.h>
#include <dirent.h>
#include <stdlib.h>
//for ionice
#include <sys/ptrace.h>
#include <asm/unistd.h>
//for getsched
#include <sched.h>

#define PROCESS_BUFFER_SIZE 1000

/* For ionice */
extern int sys_ioprio_set(int, int, int);
extern int sys_ioprio_get(int, int);

#define HAVE_IONICE
/* Check if this system has ionice */
#if !defined(SYS_ioprio_get) || !defined(SYS_ioprio_set)
/* All new kernels have SYS_ioprio_get and _set defined, but for the few that do not, here are the definitions */
#if defined(__i386__)
#define __NR_ioprio_set         289
#define __NR_ioprio_get         290
#elif defined(__ppc__) || defined(__powerpc__)
#define __NR_ioprio_set         273
#define __NR_ioprio_get         274
#elif defined(__x86_64__)
#define __NR_ioprio_set         251
#define __NR_ioprio_get         252
#elif defined(__ia64__)
#define __NR_ioprio_set         1274
#define __NR_ioprio_get         1275
#else
#ifdef __GNUC__
#warning "This architecture does not support IONICE.  Disabling ionice feature."
#endif
#undef HAVE_IONICE
#endif
/* Map these to SYS_ioprio_get */
#define SYS_ioprio_get                __NR_ioprio_get
#define SYS_ioprio_set                __NR_ioprio_set

#endif /* !SYS_ioprio_get */

/* Set up ionice functions */
#ifdef HAVE_IONICE
#define IOPRIO_WHO_PROCESS 1
#define IOPRIO_CLASS_SHIFT 13

/* Expose the kernel calls to userspace via syscall
 * See man ioprio_set  and man ioprio_get   for information on these functions */
static int ioprio_set(int which, int who, int ioprio)
{
  return syscall(SYS_ioprio_set, which, who, ioprio);
}

static int ioprio_get(int which, int who)
{
  return syscall(SYS_ioprio_get, which, who);
}
#endif




namespace KSysGuard
{

  class ProcessesLocal::Private
  {
    public:
      Private() { mProcDir = opendir( "/proc" );}
      ~Private();
      bool readProcSmaps(const QString &dir, Process *process);

      QFile mFile;
      char mBuffer[PROCESS_BUFFER_SIZE+1]; //used as a buffer to read data into
      DIR* mProcDir;
  };

ProcessesLocal::Private::~Private()
{
    closedir(mProcDir);
}

ProcessesLocal::ProcessesLocal() : d(new Private())
{

}

long ProcessesLocal::getParentPid(long pid) {
    if (pid <= 0)
        return -1;
    d->mFile.setFileName(QStringLiteral("/proc/") + QString::number(pid) + QStringLiteral("/stat"));
    if(!d->mFile.open(QIODevice::ReadOnly))
        return -1;      /* process has terminated in the meantime */

    int size; //amount of data read in
    if( (size = d->mFile.readLine( d->mBuffer, sizeof(d->mBuffer))) <= 0) { //-1 indicates nothing read
        d->mFile.close();
        return -1;
    }

    d->mFile.close();
    char *word = d->mBuffer;
    //The command name is the second parameter, and this ends with a closing bracket.  So find the last
    //closing bracket and start from there
    word = strrchr(word, ')');
    if (!word)
        return -1;
    word++; //Nove to the space after the last ")"
    int current_word = 1;

    while(true) {
        if(word[0] == ' ' ) {
            if(++current_word == 3)
                break;
        } else if(word[0] == 0) {
            return -1; //end of data - serious problem
        }
        word++;
    }
    long ppid = atol(++word);
    if (ppid == 0)
        return -1;
    return ppid;
}

bool ProcessesLocal::Private::readProcSmaps(const QString &dir, Process *process)
{
    mFile.setFileName(dir + QStringLiteral("smaps_rollup"));
    if (!mFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    auto totalPss = -1LL;
    while (mFile.readLine(mBuffer, sizeof(mBuffer)) > 0) {
        if (qstrncmp(mBuffer, "Pss:", strlen("Pss:")) == 0) {
            totalPss += atoll(mBuffer + sizeof("Pss:") - 1);
        }
    }

    mFile.close();

    process->setVmPSS(totalPss);

    return true;
}

bool ProcessesLocal::updateProcessInfo( long pid, Process *process)
{
    QString dir = QLatin1String("/proc/") + QString::number(pid) + QLatin1Char('/');

    QVector<Task*> tasks = {
        new ReadProcStatusTask(dir),
        new ReadProcCGroupTask(dir),
        new ReadProcAttrTask(dir),
        new ReadProcCmdlineTask(dir),
//         new ReadProcSmapsTask(dir),
        new ReadProcStatmTask(dir),
        new ReadProcStatTask(dir),
//         new GetNicenessTask(pid),
    };

    if (mUpdateFlags.testFlag(Processes::IOStatistics)) {
        tasks.append(new ReadProcIoTask(dir));
    }

    std::mutex mutex;
    std::condition_variable condition;
    int runningTasks = tasks.size();

    auto finished = [&mutex, &condition, &runningTasks]() {
        std::lock_guard<std::mutex> lock(mutex);
        runningTasks--;
        condition.notify_all();
    };

    for (auto task : tasks) {
        connect(task, &Task::finished, task, finished, Qt::DirectConnection);
        QThreadPool::globalInstance()->start(task);
    }

    std::unique_lock<std::mutex> lock(mutex);
    if (runningTasks > 0) {
        condition.wait(lock, [&runningTasks]() { return runningTasks == 0; });
    }

    bool success = true;
    std::for_each(tasks.begin(), tasks.end(), [&success, process](Task *task) {
        if (task->isSuccessful()) {
            task->updateProcess(process);
        } else {
            success = false;
        }
        delete task;
    });

//     success = d->readProcSmaps(dir, process);

    return success;
}

QSet<long> ProcessesLocal::getAllPids( )
{
    QSet<long> pids;
    if(d->mProcDir==nullptr) return pids; //There's not much we can do without /proc
    struct dirent* entry;
    rewinddir(d->mProcDir);
    while ( ( entry = readdir( d->mProcDir ) ) )
        if ( entry->d_name[ 0 ] >= '0' && entry->d_name[ 0 ] <= '9' )
            pids.insert(atol( entry->d_name ));
    return pids;
}

bool ProcessesLocal::sendSignal(long pid, int sig) {
    errno = 0;
    if (pid <= 0) {
        errorCode = Processes::InvalidPid;
        return false;
    }
    if (kill( (pid_t)pid, sig )) {
        switch (errno) {
            case ESRCH:
                errorCode = Processes::ProcessDoesNotExistOrZombie;
                break;
            case EINVAL:
                errorCode = Processes::InvalidParameter;
                break;
            case EPERM:
                errorCode = Processes::InsufficientPermissions;
                break;
            default:
                break;
        }
        //Kill failed
        return false;
    }
    return true;
}

bool ProcessesLocal::setNiceness(long pid, int priority) {
    errno = 0;
    if (pid <= 0) {
        errorCode = Processes::InvalidPid;
        return false;
    }
    if (setpriority( PRIO_PROCESS, pid, priority )) {
        switch (errno) {
            case ESRCH:
                errorCode = Processes::ProcessDoesNotExistOrZombie;
                break;
            case EINVAL:
                errorCode = Processes::InvalidParameter;
                break;
            case EACCES:
            case EPERM:
                errorCode = Processes::InsufficientPermissions;
                break;
            default:
                break;
        }
        //set niceness failed
        return false;
    }
    return true;
}

bool ProcessesLocal::setScheduler(long pid, int priorityClass, int priority) {
    errno = 0;
    if(priorityClass == KSysGuard::Process::Other || priorityClass == KSysGuard::Process::Batch || priorityClass == KSysGuard::Process::SchedulerIdle)
        priority = 0;
    if (pid <= 0) {
        errorCode = Processes::InvalidPid;
        return false;
    }
    struct sched_param params;
    params.sched_priority = priority;
    int policy;
    switch(priorityClass) {
      case (KSysGuard::Process::Other):
          policy = SCHED_OTHER;
          break;
      case (KSysGuard::Process::RoundRobin):
          policy = SCHED_RR;
          break;
      case (KSysGuard::Process::Fifo):
          policy = SCHED_FIFO;
          break;
#ifdef SCHED_IDLE
      case (KSysGuard::Process::SchedulerIdle):
          policy = SCHED_IDLE;
          break;
#endif
#ifdef SCHED_BATCH
      case (KSysGuard::Process::Batch):
          policy = SCHED_BATCH;
          break;
#endif
      default:
          errorCode = Processes::NotSupported;
          return false;
    }

    if (sched_setscheduler( pid, policy, &params) != 0) {
        switch (errno) {
            case ESRCH:
                errorCode = Processes::ProcessDoesNotExistOrZombie;
                break;
            case EINVAL:
                errorCode = Processes::InvalidParameter;
                break;
            case EPERM:
                errorCode = Processes::InsufficientPermissions;
                break;
            default:
                break;
        }
        return false;
    }
    return true;
}


bool ProcessesLocal::setIoNiceness(long pid, int priorityClass, int priority) {
    errno = 0;
    if (pid <= 0) {
        errorCode = Processes::InvalidPid;
        return false;
    }
#ifdef HAVE_IONICE
    if (ioprio_set(IOPRIO_WHO_PROCESS, pid, priority | priorityClass << IOPRIO_CLASS_SHIFT) == -1) {
        //set io niceness failed
        switch (errno) {
            case ESRCH:
                errorCode = Processes::ProcessDoesNotExistOrZombie;
                break;
            case EINVAL:
                errorCode = Processes::InvalidParameter;
                break;
            case EPERM:
                errorCode = Processes::InsufficientPermissions;
                break;
            default:
                break;
        }
        return false;
    }
    return true;
#else
    errorCode = Processes::NotSupported;
    return false;
#endif
}

bool ProcessesLocal::supportsIoNiceness() {
#ifdef HAVE_IONICE
    return true;
#else
    return false;
#endif
}

long long ProcessesLocal::totalPhysicalMemory() {
    //Try to get the memory via sysconf.  Note the cast to long long to try to avoid a long overflow
    //Should we use sysconf(_SC_PAGESIZE)  or getpagesize()  ?
#ifdef _SC_PHYS_PAGES
    return ((long long)sysconf(_SC_PHYS_PAGES)) * (sysconf(_SC_PAGESIZE)/1024);
#else
    //This is backup code in case this is not defined.  It should never fail on a linux system.

    d->mFile.setFileName("/proc/meminfo");
    if(!d->mFile.open(QIODevice::ReadOnly))
        return 0;

    int size;
    while( (size = d->mFile.readLine( d->mBuffer, sizeof(d->mBuffer))) > 0) {  //-1 indicates an error
        switch( d->mBuffer[0]) {
	  case 'M':
            if((unsigned int)size > sizeof("MemTotal:") && qstrncmp(d->mBuffer, "MemTotal:", sizeof("MemTotal:")-1) == 0) {
		    d->mFile.close();
		    return atoll(d->mBuffer + sizeof("MemTotal:")-1);
            }
	}
    }
    return 0; // Not found.  Probably will never happen
#endif
}
ProcessesLocal::~ProcessesLocal()
{
  delete d;
}

}
