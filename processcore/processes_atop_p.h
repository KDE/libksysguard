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

#ifndef PROCESSES_ATOP_H_
#define PROCESSES_ATOP_H_

#include "processes_base_p.h"
#include <unistd.h>  //For sysconf


#include <QSet>
class QDateTime;

namespace KSysGuard
{
    class Process;

    /**
     * This is the ATOP specific code to get process information for the local host.
     */
    class ProcessesATop : public AbstractProcesses {
        public:
            explicit ProcessesATop(bool loadDefaultFile = true);
            ~ProcessesATop() override;
            QSet<long> getAllPids() override;
            long getParentPid(long pid) override;
            bool updateProcessInfo(long pid, Process *process) override;
            bool sendSignal(long pid, int sig) override;
            bool setNiceness(long pid, int priority) override;
            bool setScheduler(long pid, int priorityClass, int priority) override;
            long long totalPhysicalMemory() override;
            bool setIoNiceness(long pid, int priorityClass, int priority) override;
            bool supportsIoNiceness() override;
            long numberProcessorCores() override
#ifdef _SC_NPROCESSORS_ONLN
            { return sysconf(_SC_NPROCESSORS_ONLN); } // Should work on any recent posix system
#else
            ;
#endif
            void updateAllProcesses(Processes::UpdateFlags updateFlags) override { mUpdateFlags = updateFlags; emit processesUpdated(); } //For local machine, there is no delay

            bool isHistoryAvailable() const;
            QDateTime viewingTime() const;
            bool setViewingTime(const QDateTime &when);
            QList< QPair<QDateTime, uint> > historiesAvailable() const;
            bool loadHistoryFile(const QString &filename);
            QString historyFileName() const;
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
