/*
    SPDX-FileCopyrightText: 2007 Manolo Valdes <nolis71cu@gmail.com>
    SPDX-FileCopyrightText: 2025 Rafael Sadowski <rafael@rsadowski.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "process.h"
#include "processes_local_p.h"

#include "memoryinfo_p.h"

#include <KLocalizedString>

#include <QSet>

#include <sys/param.h>
#include <sys/resource.h>
#include <uvm/uvmexp.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

namespace KSysGuard
{
class ProcessesLocal::Private
{
public:
    Private()
    {
        ;
    }
    ~Private()
    {
        ;
    }
    inline bool readProc(long pid, struct kinfo_proc *p);
    inline void readProcStatus(struct kinfo_proc *p, Process *process);
    inline void readProcStat(struct kinfo_proc *p, Process *process);
    inline bool readProcCmdline(long pid, Process *process);
};

bool ProcessesLocal::Private::readProc(long pid, struct kinfo_proc *p)
{
    int mib[4];
    size_t len;

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = pid;

    len = sizeof(struct kinfo_proc);
    if (sysctl(mib, 4, p, &len, NULL, 0) == -1 || !len) {
        return false;
    }
    return true;
}

void ProcessesLocal::Private::readProcStatus(struct kinfo_proc *p, Process *process)
{
    process->setEuid(p->p_uid);
    process->setUid(p->p_ruid);
    process->setEgid(p->p_gid);
    process->setGid(p->p_rgid);
    process->setTracerpid(-1);
    process->setName(QString::fromUtf8(p->p_comm[0] ? p->p_comm : "????"));
}

void ProcessesLocal::Private::readProcStat(struct kinfo_proc *p, Process *ps)
{
    int status;
    ps->setUserTime(p->p_uutime_sec * 100 + p->p_uutime_usec / 10000);
    ps->setSysTime(p->p_ustime_sec * 100 + p->p_ustime_usec / 10000);
    ps->setNiceLevel(p->p_nice);

    status = p->p_stat;

    MemoryFields fields;
    fields.rss = p->p_vm_rssize * getpagesize() / 1024;
    fields.lastUpdate = std::chrono::steady_clock::now();
    ps->memoryInfo()->imprecise = fields;
    ps->memoryInfo()->vmSize = p->p_vm_map_size / 1024;
    ps->addChange(Process::Memory);


    // "idle","run","sleep","stop","zombie"
    switch (status) {
    case SRUN:
        ps->setStatus(Process::Running);
        break;
    case SSLEEP:
        ps->setStatus(Process::Sleeping);
        break;
    case SSTOP:
        ps->setStatus(Process::Stopped);
        break;
    case SZOMB:
        ps->setStatus(Process::Zombie);
        break;
    default:
        ps->setStatus(Process::OtherStatus);
        break;
    }
}

bool ProcessesLocal::Private::readProcCmdline(long pid, Process *process)
{
    int mib[4];
    size_t buflen = 256;
    char buf[256];

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_ARGS;
    mib[3] = pid;

    if (sysctl(mib, 4, buf, &buflen, NULL, 0) == -1 || !buflen) {
        return false;
    }
    QString command = QString(buf);

    // cmdline separates parameters with the NULL character
    command.replace('\0', ' ');
    process->setCommand(command.trimmed());

    return true;
}

ProcessesLocal::ProcessesLocal()
    : d(new Private())
{
}

long ProcessesLocal::getParentPid(long pid)
{
    Q_ASSERT(pid != 0);
    long long ppid = -1;
    struct kinfo_proc p;
    if (d->readProc(pid, &p)) {
        ppid = p.p_ppid;
    }
    return ppid;
}

bool ProcessesLocal::updateProcessInfo(long pid, Process *process)
{
    struct kinfo_proc p;
    if (!d->readProc(pid, &p)) {
        return false;
    }
    d->readProcStat(&p, process);
    d->readProcStatus(&p, process);
    if (!d->readProcCmdline(pid, process)) {
        return false;
    }

    return true;
}

QSet<long> ProcessesLocal::getAllPids()
{
    QSet<long> pids;
    int mib[3];
    size_t len;
    size_t num;
    struct kinfo_proc *p;

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_ALL;
    if (sysctl(mib, 3, NULL, &len, NULL, 0) == -1) {
        return pids;
    }
    if ((p = (kinfo_proc *)malloc(len)) == NULL) {
        return pids;
    }
    if (sysctl(mib, 3, p, &len, NULL, 0) == -1) {
        free(p);
        return pids;
    }

    for (num = 0; num < len / sizeof(struct kinfo_proc); num++) {
        long pid = p[num].p_pid;
        long long ppid = p[num].p_ppid;

        // skip all process with parent id = 0 but init
        if (ppid <= 0 && pid != 1) {
            continue;
        }
        pids.insert(pid);
    }
    free(p);
    return pids;
}

Processes::Error ProcessesLocal::sendSignal(long pid, int sig)
{
    if (kill((pid_t)pid, sig)) {
        // Kill failed
        return Processes::Unknown;
    }
    return Processes::NoError;
}

Processes::Error ProcessesLocal::setNiceness(long pid, int priority)
{
    if (setpriority(PRIO_PROCESS, pid, priority)) {
        // set niceness failed
        return Processes::Unknown;
    }
    return Processes::NoError;
}

int ProcessesLocal::getNiceness(long pid)
{
    errno = 0;
    const int nice = getpriority(PRIO_PROCESS, pid);
    if (errno != 0) {
        return 0;
    }
    return nice;
}

Processes::Error ProcessesLocal::setScheduler(long pid, int priorityClass, int priority)
{
    Q_UNUSED(priority)
    Q_UNUSED(priorityClass)
    if (pid <= 0) {
        return Processes::InvalidPid; // check the parameters
    }
    // OpenBSD doesn't support sched_setscheduler
    return Processes::NotSupported;
}

int ProcessesLocal::getSchedulerClass(long pid)
{
    Q_UNUSED(pid)
    return 0;
}

Processes::Error ProcessesLocal::setIoNiceness(long pid, int priorityClass, int priority)
{
    Q_UNUSED(pid)
    Q_UNUSED(priority)
    Q_UNUSED(priorityClass)
    return Processes::NotSupported; // Not yet supported
}

int ProcessesLocal::getIoNiceness(long pid)
{
    Q_UNUSED(pid)
    return 0;
}

int ProcessesLocal::getIoPriorityClass(long pid)
{
    Q_UNUSED(pid)
    return KSysGuard::Process::None;
}

bool ProcessesLocal::supportsIoNiceness()
{
    return false;
}

long long ProcessesLocal::totalPhysicalMemory()
{
    int64_t physmem = 0;
    int mib[] = {CTL_HW, HW_PHYSMEM64};
    size_t sz = sizeof(physmem);
    if (sysctl(mib, 2, &physmem, &sz, NULL, 0) == 0) {
        return physmem / 1024;
    }
    return 0;
}

long long ProcessesLocal::totalSwapMemory()
{
    struct uvmexp uvmexp;
    int mib[] = {CTL_VM, VM_UVMEXP};
    size_t sz = sizeof(uvmexp);
    if (sysctl(mib, 2, &uvmexp, &sz, NULL, 0) == 0) {
        return (int64_t)uvmexp.swpages * uvmexp.pagesize / 1024;
    }
    return 0;
}

ProcessesLocal::~ProcessesLocal()
{
    delete d;
}
}
