/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "GetNicenessTask.h"

#include <unistd.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/syscall.h>

#define IOPRIO_WHO_PROCESS 1

using namespace KSysGuard;

GetNicenessTask::GetNicenessTask(pid_t pid)
    : Task(QString{})
    , m_pid(pid)
{
}

void GetNicenessTask::updateProcess(KSysGuard::Process* process)
{
    process->setScheduler(m_scheduler);
    process->setNiceLevel(m_priority);
    process->setIoniceLevel(m_ioPriority);
    process->setIoPriorityClass(m_ioScheduler);
}

void KSysGuard::GetNicenessTask::run()
{
    Q_EMIT started();

    int sched = sched_getscheduler(m_pid);

    switch(sched) {
        case (SCHED_OTHER):
            m_scheduler = Process::Other;
            break;
        case (SCHED_RR):
            m_scheduler = Process::RoundRobin;
            break;
        case (SCHED_FIFO):
            m_scheduler = Process::Fifo;
            break;
        case (SCHED_IDLE):
            m_scheduler = Process::SchedulerIdle;
            break;
        case (SCHED_BATCH):
            m_scheduler = Process::Batch;
            break;
        default:
            m_scheduler = Process::Other;
    }

    if (sched == SCHED_FIFO || sched == SCHED_RR) {
        struct sched_param param;
        if(sched_getparam(m_pid, &param) == 0) {
            m_priority = param.sched_priority;
        }
    } else {
        m_priority = getpriority(PRIO_PROCESS, m_pid);
    }

    int ioprio = syscall(SYS_ioprio_get, IOPRIO_WHO_PROCESS, m_pid);
    if (ioprio == -1) {
        return;
    }

    m_ioPriority = ioprio & 0xff;
    m_ioScheduler = static_cast<Process::IoPriorityClass>(ioprio >> 13);

    setFinished(true);
    Q_EMIT finished();
}

bool GetNicenessTask::processLine(const QByteArray& data)
{
    Q_UNUSED(data)
    return true;
}
