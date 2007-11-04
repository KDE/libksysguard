/*  This file is part of the KDE project
    Copyright (C) 2007 John Tapsell <tapsell@kde.org>

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

#ifndef PROCESS_H_
#define PROCESS_H_

#include <kdemacros.h>

#include <QtCore/QList>
#include <QtCore/QTime>

#include <klocale.h>


namespace KSysGuard
{

  class KDE_EXPORT Process {
    public:
	enum ProcessStatus { Running, Sleeping, DiskSleep, Zombie, Stopped, Paging, OtherStatus };
	enum IoPriorityClass { None, RealTime, BestEffort, Idle };
	enum Scheduler { Other = 0, Fifo, RoundRobin, Batch };
        Process();
        Process(long long _pid, long long _ppid, Process *_parent);

        long pid;    ///The system's ID for this process.  1 for init.  0 for our virtual 'parent of init' process used just for convience.
        long parent_pid;  ///The system's ID for the parent of this process.  0 for init.

        /** A guaranteed NON-NULL pointer for all real processes to the parent process except for the fake process with pid 0.
         *  The Parent's pid is the same value as the parent_pid.  The parent process will be also pointed
         *  to by ProcessModel::mPidToProcess to there is no need to worry about mem management in using parent.
         *  For init process, parent will point to a (fake) process with pid 0 to simplify things.
	 *  For the fake process, this will point to NULL
         */
        Process *parent;

        long long uid; ///The user id that the process is running as
        long long euid; ///The effective user id that the process is running as
        long long suid; ///The set user id that the process is running as
        long long fsuid; ///The file system user id that the process is running as.

        long long gid; ///The process group id that the process is running as
        long long egid; ///The effective group id that the process is running as
        long long sgid; ///The set group id that the process is running as
        long long fsgid; ///The file system group id that the process is running as

        long long tracerpid; ///If this is being debugged, this is the process that is debugging it
	QByteArray tty; /// The name of the tty the process owns
        long long userTime; ///The time, in 100ths of a second, spent in total on user calls. -1 if not known
        long long sysTime;  ///The time, in 100ths of a second, spent in total on system calls.  -1 if not known
        int userUsage; ///Percentage (0 to 100).  It might be more than 100% on multiple cpu core systems
        int sysUsage;  ///Percentage (0 to 100).  It might be more than 100% on multiple cpu core systems
        int totalUserUsage; ///Percentage (0 to 100) from the sum of itself and all its children recursively.  If there's no children, it's equal to userUsage.  It might be more than 100% on multiple cpu core systems
        int totalSysUsage; ///Percentage (0 to 100) from the sum of itself and all its children recursively. If there's no children, it's equal to sysUsage. It might be more than 100% on multiple cpu core systems
        unsigned long numChildren; ///Number of children recursively that this process has.  From 0+
        int niceLevel;      ///If Scheduler = Other, niceLevel is the niceness (-20 to 20) of this process.  A lower number means a higher priority.  Otherwise sched priority (1 to 99)
	Scheduler scheduler; ///The scheduler this process is running in.  See man sched_getscheduler for more info
        IoPriorityClass ioPriorityClass; /// The IO priority class.  See man ionice for detailed information.
        int ioniceLevel;    ///IO Niceness (0 to 7) of this process.  A lower number means a higher io priority.  -1 if not known or not applicable because ioPriorityClass is Idle or None
        long vmSize;   ///Virtual memory size in KiloBytes, including memory used, mmap'ed files, graphics memory etc,
        long vmRSS;    ///Physical memory used by the process and its shared libraries.  If the process and libraries are swapped to disk, this could be as low as 0
        long vmURSS;   ///Physical memory used only by the process, and not counting the code for shared libraries. Set to -1 if unknown
        QString name;  ///The name (e.g. "ksysguard", "konversation", "init")
        QString command; ///The command the process was launched with
        QList<Process *> children;  ///A list of all the direct children that the process has.  Children of children are not listed here, so note that children_pids <= numChildren
	ProcessStatus status; ///Whether the process is running/sleeping/etc
	QTime timeKillWasSent; /// This is usually a NULL time.  When trying to kill a process, this is the time that the kill signal was sent to the process.

	QString translatedStatus() const;  /// Returns a translated string of the status. e.g. "Running" etc
	QString niceLevelAsString() const; /// Returns a simple translated string of the nice priority.  e.g. "Normal", "High", etc
	QString ioniceLevelAsString() const; /// Returns a simple translated string of the io nice priority.  e.g. "Normal", "High", etc
	QString ioPriorityClassAsString() const; /// Returns a translated string of the io nice class.  i.e. "None", "Real Time", "Best Effort", "Idle"
	QString schedulerAsString() const; /// Returns a translated string of the scheduler class.  e.g. "FIFO", "Round Robin", "Batch"
        
	int index;  /// Each process has a parent process.  Each sibling has a unique number to identify it under that parent.  This is that number.

  private:
        void clear();
    }; 

}

#endif
