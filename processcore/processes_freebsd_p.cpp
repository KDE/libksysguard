/*  This file is part of the KDE project
    Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

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

#include <klocale.h>

#include <QSet>

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/resource.h>
#if defined(__DragonFly__)
#include <sys/resourcevar.h>
#include <err.h>
#endif
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>



namespace KSysGuard
{

  class ProcessesLocal::Private
  {
    public:
      Private() {;}
      ~Private() {;}
      inline bool readProc(long pid, struct kinfo_proc *p);
      inline void readProcStatus(struct kinfo_proc *p, Process *process);
      inline void readProcStat(struct kinfo_proc *p, Process *process);
      inline void readProcStatm(struct kinfo_proc *p, Process *process);
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

    len = sizeof (struct kinfo_proc);
    if (sysctl(mib, 4, p, &len, NULL, 0) == -1 || !len)
        return false;
    return true;
}

void ProcessesLocal::Private::readProcStatus(struct kinfo_proc *p, Process *process)
{
    process->uid = 0;
    process->gid = 0;
    process->tracerpid = 0;


#if defined(__FreeBSD__) && __FreeBSD_version >= 500015
    process->uid  = p->ki_uid;    
    process->gid  = p->ki_pgid;
    process->name = QString(p->ki_comm ? p->ki_comm : "????");
#elif defined(__DragonFly__) && __DragonFly_version >= 190000
    process->uid  = p->kp_uid;
    process->gid  = p->kp_pgid;
    process->name = QString(p->kp_comm ? p->kp_comm : "????");
#else
    process->uid  = p->kp_eproc.e_ucred.cr_uid;
    process->gid  = p->kp_eproc.e_pgid;
#endif
}

void ProcessesLocal::Private::readProcStat(struct kinfo_proc *p, Process *ps)
{
    int status;
    struct rusage pru;
#if defined(__FreeBSD__) && __FreeBSD_version >= 500015
        ps->userTime  = p->ki_runtime / 10000;
        ps->niceLevel = p->ki_nice;
        ps->vmSize    = p->ki_size;
        ps->vmRSS     = p->ki_rssize * getpagesize();
        status = p->ki_stat;
#elif defined(__DragonFly__) && __DragonFly_version >= 190000
        if (!getrusage(p->kp_pid, &pru)) {
            errx(1, "failed to get rusage info");
        }
        ps->userTime  = pru.ru_utime.tv_usec / 1000; /*p_runtime / 1000*/
        ps->niceLevel = p->kp_nice;
        ps->vmSize    = p->kp_vm_map_size;
        ps->vmRSS     = p->kp_vm_rssize * getpagesize();
        status = p->kp_stat;
#else
        ps->userTime  = p->kp_proc.p_rtime.tv_sec*100+p->kp_proc.p_rtime.tv_usec/100;
        ps->niceLevel = p->kp_proc.p_nice;
        ps->vmSize    = p->kp_eproc.e_vm.vm_map.size;
        ps->vmRSS     = p->kp_eproc.e_vm.vm_rssize * getpagesize();
        status = p->kp_proc.p_stat;
#endif
        ps->sysTime   = 0;

// "idle","run","sleep","stop","zombie"
    switch( status ) {
      case '0':
        ps->status = Process::DiskSleep;
	break;
      case '1':
        ps->status = Process::Running;
	break;
      case '2':
        ps->status = Process::Sleeping;
	break;
      case '3':
        ps->status = Process::Stopped;
	break;
      case '4':
         ps->status = Process::Zombie;
         break;
      default:
         ps->status = Process::OtherStatus;
         break;
    }
}

void ProcessesLocal::Private::readProcStatm(struct kinfo_proc *p, Process *process)
{
// TODO

//     unsigned long shared;    
//     process->vmURSS = process->vmRSS - (shared * sysconf(_SC_PAGESIZE) / 1024);
}

bool ProcessesLocal::Private::readProcCmdline(long pid, Process *process)
{
    int mib[4];
    struct kinfo_proc p;
    size_t buflen = 256;
    char buf[256];

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_ARGS;
    mib[3] = pid;

    if (sysctl(mib, 4, buf, &buflen, NULL, 0) == -1 || !buflen)
        return false;
    process->command = QString(buf);

    //cmdline seperates parameters with the NULL character
    process->command.replace('\0', ' ');
    process->command = process->command.trimmed();

    return true;
}

ProcessesLocal::ProcessesLocal() : d(new Private())
{

}

long ProcessesLocal::getParentPid(long pid) {
    long long ppid = 0;
    struct kinfo_proc p;
    if(d->readProc(pid, &p))
    {
#if defined(__FreeBSD__) && __FreeBSD_version >= 500015
        ppid = p.ki_ppid;
#elif defined(__DragonFly__) && __DragonFly_version >= 190000
        ppid = p.kp_ppid;
#else
        ppid = p.kp_eproc.e_ppid;
#endif
    }
    return ppid;
}

bool ProcessesLocal::updateProcessInfo( long pid, Process *process)
{
    struct kinfo_proc p;
    if(!d->readProc(pid, &p)) return false;
    d->readProcStat(&p, process);
    d->readProcStatus(&p, process);
    d->readProcStatm(&p, process);
    if(!d->readProcCmdline(pid, process)) return false;

    return true;
}

QSet<long> ProcessesLocal::getAllPids( )
{
    QSet<long> pids;
    int mib[3];
    size_t len;
    size_t num;
    struct kinfo_proc *p;

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_ALL;
    sysctl(mib, 3, NULL, &len, NULL, 0);
    p = (kinfo_proc *) malloc(len);
    sysctl(mib, 3, p, &len, NULL, 0);

    for (num = 0; num < len / sizeof(struct kinfo_proc); num++)
    {
#if defined(__FreeBSD__) && __FreeBSD_version >= 500015
        long pid = p[num].ki_pid;
        long long ppid = p[num].ki_ppid;
#elif defined(__DragonFly__) && __DragonFly_version >= 190000
        long pid = p[num].kp_pid;
        long long ppid = p[num].kp_ppid;
#else
        long pid = p[num].kp_proc.p_pid;
        long long ppid = p[num].kp_eproc.e_ppid;
#endif
        //skip all process with parent id = 0 but init
        if(ppid == 0 && pid != 1)
            continue;
        pids.insert(pid);
    }
    free(p);
    return pids;
}

bool ProcessesLocal::sendSignal(long pid, int sig) {
    if ( kill( (pid_t)pid, sig ) ) {
	//Kill failed
        return false;
    }
    return true;
}

bool ProcessesLocal::setNiceness(long pid, int priority) {
    if ( setpriority( PRIO_PROCESS, pid, priority ) ) {
	    //set niceness failed
	    return false;
    }
    return true;
}

bool ProcessesLocal::setScheduler(long pid, int priorityClass, int priority) 
{
    if(priorityClass == KSysGuard::Process::Other || priorityClass == KSysGuard::Process::Batch)
	    priority = 0;
    if(pid <= 0) return false; // check the parameters
    struct sched_param params;
    params.sched_priority = priority;
    switch(priorityClass) {
      case (KSysGuard::Process::Other):
	    return (sched_setscheduler( pid, SCHED_OTHER, &params) == 0);
      case (KSysGuard::Process::RoundRobin):
	    return (sched_setscheduler( pid, SCHED_RR, &params) == 0);
      case (KSysGuard::Process::Fifo):
	    return (sched_setscheduler( pid, SCHED_FIFO, &params) == 0);
#ifdef SCHED_BATCH
      case (KSysGuard::Process::Batch):
	    return (sched_setscheduler( pid, SCHED_BATCH, &params) == 0);
#endif
      default:
	    return false;
    }
}

bool ProcessesLocal::setIoNiceness(long pid, int priorityClass, int priority) {
    return false; //Not yet supported
}

bool ProcessesLocal::supportsIoNiceness() {
    return false;
}

long long ProcessesLocal::totalPhysicalMemory() {

    size_t Total;
    size_t len;
    len = sizeof (Total);
    sysctlbyname("hw.physmem", &Total, &len, NULL, 0);
    return Total /= 1024;
}

ProcessesLocal::~ProcessesLocal()
{
  
}

}
