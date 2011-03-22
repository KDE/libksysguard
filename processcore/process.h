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

#ifndef PROCESS_H_
#define PROCESS_H_

#include <kdemacros.h>

#include <QtCore/QList>
#include <QtCore/QTime>
#include <QtCore/QFlags>

#include <klocale.h>

namespace KSysGuard
{

  class KDE_EXPORT Process {
    public:
    enum ProcessStatus { Running, Sleeping, DiskSleep, Zombie, Stopped, Paging, Ended, OtherStatus=99 };
    enum IoPriorityClass { None, RealTime, BestEffort, Idle };
    enum Scheduler { Other = 0, Fifo, RoundRobin, Batch, SchedulerIdle, Interactive }; ///< Interactive is Solaris only
    Process();
    Process(qlonglong _pid, qlonglong _ppid, Process *_parent);

    long pid;    ///< The system's ID for this process.  1 for init.  -1 for our virtual 'parent of init' process used just for convenience.
    long parent_pid;  ///< The system's ID for the parent of this process.  Set to -1 if it has no parent (e.g. 'init' on Linux).

    /** A guaranteed NON-NULL pointer for all real processes to the parent process except for the fake process with pid -1.
     *  The Parent's pid is the same value as the parent_pid.  The parent process will be also pointed
     *  to by ProcessModel::mPidToProcess to there is no need to worry about mem management in using parent.
     *  For process without a parent (such as 'init' on Linux, parent will point to a (fake) process with pid -1 to simplify things.
     *  For the fake process, this will point to NULL
     */
    Process *parent;

    void setNumThreads(int number); ///< The number of threads that this process has, including this process.
    void setLogin(QString login); ///< The user login name.  Only used for processes on remote machines.  Otherwise use uid to get the name
    void setUid(qlonglong uid); ///< The user id that the process is running as
    void setEuid(qlonglong euid); ///< The effective user id that the process is running as
    void setSuid(qlonglong suid); ///< The set user id that the process is running as
    void setFsuid(qlonglong fsuid); ///< The file system user id that the process is running as.

    void setGid(qlonglong gid); ///< The process group id that the process is running as
    void setEgid(qlonglong egid); ///< The effective group id that the process is running as
    void setSgid(qlonglong sgid); ///< The set group id that the process is running as
    void setFsgid(qlonglong fsgid); ///< The file system group id that the process is running as

    void setTracerpid(qlonglong tracerpid); ///< If this is being debugged, this is the process that is debugging it, or 0 otherwise
    void setTty(QByteArray tty); ///< The name of the tty the process owns
    void setUserTime(qlonglong userTime); ///< The time, in 100ths of a second, spent in total on user calls. -1 if not known
    void setSysTime(qlonglong sysTime);  ///< The time, in 100ths of a second, spent in total on system calls.  -1 if not known
    void setUserUsage(int userUsage); ///< Percentage (0 to 100).  It might be more than 100% on multiple cpu core systems
    void setSysUsage(int sysUsage);  ///< Percentage (0 to 100).  It might be more than 100% on multiple cpu core systems
    void setTotalUserUsage(int totalUserUsage); ///< Percentage (0 to 100) from the sum of itself and all its children recursively.  If there's no children, it's equal to userUsage.  It might be more than 100% on multiple cpu core systems
    void setTotalSysUsage(int totalSysUsage); ///< Percentage (0 to 100) from the sum of itself and all its children recursively. If there's no children, it's equal to sysUsage. It might be more than 100% on multiple cpu core systems
    void setNiceLevel(int niceLevel);      ///< If Scheduler = Other, niceLevel is the niceness (-20 to 20) of this process.  A lower number means a higher priority.  Otherwise sched priority (1 to 99)
    void setscheduler(Scheduler scheduler); ///< The scheduler this process is running in.  See man sched_getscheduler for more info
    void setIoPriorityClass(IoPriorityClass ioPriorityClass); ///< The IO priority class.  See man ionice for detailed information.
    void setIoniceLevel(int ioniceLevel);    ///< IO Niceness (0 to 7) of this process.  A lower number means a higher io priority.  -1 if not known or not applicable because ioPriorityClass is Idle or None
    void setVmSize(qlonglong vmSize);   ///< Virtual memory size in KiloBytes, including memory used, mmap'ed files, graphics memory etc,
    void setVmRSS(qlonglong vmRSS);    ///< Physical memory used by the process and its shared libraries.  If the process and libraries are swapped to disk, this could be as low as 0
    void setVmURSS(qlonglong vmURSS);   ///< Physical memory used only by the process, and not counting the code for shared libraries. Set to -1 if unknown
    void setName(QString name);  ///< The name (e.g. "ksysguard", "konversation", "init")
    void setCommand(QString command); ///< The command the process was launched with
    void setStatus( ProcessStatus status); ///< Whether the process is running/sleeping/etc

    void setIoCharactersRead(qlonglong number); ///< The number of bytes which this task has caused to be read from storage
    void setIoCharactersWritten(qlonglong number); ///< The number of bytes which this task has caused, or shall cause to be written to disk.
    void setIoReadSyscalls(qlonglong number); ///< Number of read I/O operations, i.e. syscalls like read() and pread().
    void setIoWriteSyscalls(qlonglong number); ///< Number of write I/O operations, i.e. syscalls like write() and pwrite().
    void setIoCharactersActuallyRead(qlonglong number); ///< Number of bytes which this process really did cause to be fetched from the storage layer.
    void setIoCharactersActuallyWritten(qlonglong number); ///< Attempt to count the number of bytes which this process caused to be sent to the storage layer.

    void setIoCharactersReadRate(long number); ///< The rate, in bytes per second, which this task has caused to be read from storage
    void setIoCharactersWrittenRate(long number); ///< The rate, in bytes per second, which this task has caused, or shall cause to be written to disk.
    void setIoReadSyscallsRate(long number); ///< Number of read I/O operations per second, i.e. syscalls like read() and pread().
    void setIoWriteSyscallsRate(long number); ///< Number of write I/O operations per second, i.e. syscalls like write() and pwrite().
    void setIoCharactersActuallyReadRate(long number); ///< Number of bytes per second which this process really did cause to be fetched from the storage layer.
    void setIoCharactersActuallyWrittenRate(long number); ///< Attempt to count the number of bytes per second which this process caused to be sent to the storage layer.

    /* The member variables are made to public for efficiency, but should only be read from. */
    QString login;
    qlonglong uid;
    qlonglong euid;
    qlonglong suid;
    qlonglong fsuid;

    qlonglong gid;
    qlonglong egid;
    qlonglong sgid;
    qlonglong fsgid;

    qlonglong tracerpid;
    QByteArray tty;
    qlonglong userTime;
    qlonglong sysTime;
    int userUsage;
    int sysUsage;
    int totalUserUsage;
    int totalSysUsage;
    unsigned long numChildren;
    int niceLevel;
    Scheduler scheduler;
    IoPriorityClass ioPriorityClass;
    int ioniceLevel;

    qlonglong vmSize;
    qlonglong vmRSS;
    qlonglong vmURSS;

    qlonglong vmSizeChange; ///< The change in vmSize since last update, in KiB
    qlonglong vmRSSChange;  ///< The change in vmRSS since last update, in KiB
    qlonglong vmURSSChange; ///< The change in vmURSS since last update, in KiB

    unsigned long pixmapBytes; ///< The number of bytes used for pixmaps/images and not counted by vmRSS or vmURSS
    bool hasManagedGuiWindow;
    QString name;
    QString command;
    ProcessStatus status;
    qlonglong ioCharactersRead;
    qlonglong ioCharactersWritten;
    qlonglong ioReadSyscalls;
    qlonglong ioWriteSyscalls;
    qlonglong ioCharactersActuallyRead;
    qlonglong ioCharactersActuallyWritten;

    long ioCharactersReadRate;
    long ioCharactersWrittenRate;
    long ioReadSyscallsRate;
    long ioWriteSyscallsRate;
    long ioCharactersActuallyReadRate;
    long ioCharactersActuallyWrittenRate;

    QList<Process *> children;  ///< A list of all the direct children that the process has.  Children of children are not listed here, so note that children_pids <= numChildren
    QTime timeKillWasSent; ///< This is usually a NULL time.  When trying to kill a process, this is the time that the kill signal was sent to the process.

    QString translatedStatus() const;  ///< Returns a translated string of the status. e.g. "Running" etc
    QString niceLevelAsString() const; ///< Returns a simple translated string of the nice priority.  e.g. "Normal", "High", etc
    QString ioniceLevelAsString() const; ///< Returns a simple translated string of the io nice priority.  e.g. "Normal", "High", etc
    QString ioPriorityClassAsString() const; ///< Returns a translated string of the io nice class.  i.e. "None", "Real Time", "Best Effort", "Idle"
    QString schedulerAsString() const; ///< Returns a translated string of the scheduler class.  e.g. "FIFO", "Round Robin", "Batch"

    int index;  ///< Each process has a parent process.  Each sibling has a unique number to identify it under that parent.  This is that number.

    /** An enum to keep track of what changed since the last update.  Note that we
     * the maximum we can use is 0x4000, so some of the enums represent multiple variables
     */
    enum Change {
        Nothing = 0x0,
        Uids = 0x1,
        Gids = 0x2,
        Tracerpid = 0x4,
        Tty = 0x8,
        Usage = 0x10,
        TotalUsage = 0x20,
        NiceLevels = 0x40,
        VmSize = 0x80,
        VmRSS = 0x100,
        VmURSS = 0x200,
        Name = 0x400,
        Command = 0x800,
        Status = 0x1000,
        Login = 0x2000,
        IO = 0x4000,
        NumThreads = 0x8000
    };
    Q_DECLARE_FLAGS(Changes, Change)

    Changes changes;  /**< A QFlags representing what has changed */

    /** This is the number of 1/1000ths of a second since this
     *  particular process was last updated compared to when all the processes
     *  were updated. The purpose is to allow a more fine tracking of the time
     *  a process has been running for.
     *
     *  This is updated in processes.cpp and so shouldn't be touched by the
     *  OS dependant classes.
     */
    int elapsedTimeMilliSeconds;

    int numThreads; ///< Number of threads that this process has, including the main one.  0 if not known

  private:
    void clear();

  };
  Q_DECLARE_OPERATORS_FOR_FLAGS(Process::Changes)
}

#endif
