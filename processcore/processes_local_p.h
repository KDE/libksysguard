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

#ifndef PROCESSES_LOCAL_H_
#define PROCESSES_LOCAL_H_

#include "processes_base_p.h"
#include <unistd.h>  //For sysconf


#include <QSet>

namespace KSysGuard
{
    class Process;

    /**
     * This is the OS specific code to get process information for the local host.
     */
    class ProcessesLocal : public AbstractProcesses {
        public:
            ProcessesLocal();
            virtual ~ProcessesLocal();
            QSet<long> getAllPids() Q_DECL_OVERRIDE;
            long getParentPid(long pid) Q_DECL_OVERRIDE;
            bool updateProcessInfo(long pid, Process *process) Q_DECL_OVERRIDE;
            bool sendSignal(long pid, int sig) Q_DECL_OVERRIDE;
            bool setNiceness(long pid, int priority) Q_DECL_OVERRIDE;
            bool setScheduler(long pid, int priorityClass, int priority) Q_DECL_OVERRIDE;
            long long totalPhysicalMemory() Q_DECL_OVERRIDE;
            bool setIoNiceness(long pid, int priorityClass, int priority) Q_DECL_OVERRIDE;
            bool supportsIoNiceness() Q_DECL_OVERRIDE;
            long numberProcessorCores() Q_DECL_OVERRIDE
#ifdef _SC_NPROCESSORS_ONLN
            { return sysconf(_SC_NPROCESSORS_ONLN); } // Should work on any recent posix system
#else
            ;
#endif
            void updateAllProcesses(Processes::UpdateFlags updateFlags) Q_DECL_OVERRIDE { mUpdateFlags = updateFlags; emit processesUpdated(); } //For local machine, there is no delay

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
