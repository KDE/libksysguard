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

#include "process.h"
#include <KLocalizedString>

class KSysGuard::ProcessPrivate {
public:
    long pid;

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
    qlonglong startTime;
};

KSysGuard::Process::Process()
    : d(new ProcessPrivate())
{
    clear();
}

KSysGuard::Process::Process(qlonglong _pid, qlonglong _ppid, Process *_parent)
    : d(new ProcessPrivate())
{
    clear();
    d->pid = _pid;
    parent_pid = _ppid;
    parent = _parent;
}

KSysGuard::Process::~Process()
{
    delete d;
}

QString KSysGuard::Process::niceLevelAsString() const {
    // Just some rough heuristic to map a number to how nice it is
    if( niceLevel == 0) return i18nc("Process Niceness", "Normal");
    if( niceLevel >= 10) return i18nc("Process Niceness", "Very low priority");
    if( niceLevel > 0) return i18nc("Process Niceness", "Low priority");
    if( niceLevel <= -10) return i18nc("Process Niceness", "Very high priority");
    if( niceLevel < 0) return i18nc("Process Niceness", "High priority");
    return QString(); //impossible;
}

QString KSysGuard::Process::ioniceLevelAsString() const {
    // Just some rough heuristic to map a number to how nice it is
    if( ioniceLevel == 4) return i18nc("Process Niceness", "Normal");
    if( ioniceLevel >= 6) return i18nc("Process Niceness", "Very low priority");
    if( ioniceLevel > 4) return i18nc("Process Niceness", "Low priority");
    if( ioniceLevel <= 2) return i18nc("Process Niceness", "Very high priority");
    if( ioniceLevel < 4) return i18nc("Process Niceness", "High priority");
    return QString(); //impossible;

}

QString KSysGuard::Process::ioPriorityClassAsString() const {
    switch( ioPriorityClass ) {
        case None: return i18nc("Priority Class", "None");
        case RealTime: return i18nc("Priority Class", "Real Time");
        case BestEffort: return i18nc("Priority Class", "Best Effort");
        case Idle: return i18nc("Priority Class", "Idle");
        default: return i18nc("Priority Class", "Unknown");
    }
}

QString KSysGuard::Process::translatedStatus() const {
    switch( status ) {
        case Running: return i18nc("process status", "running");
        case Sleeping: return i18nc("process status", "sleeping");
        case DiskSleep: return i18nc("process status", "disk sleep");
        case Zombie: return i18nc("process status", "zombie");
        case Stopped: return i18nc("process status", "stopped");
        case Paging: return i18nc("process status", "paging");
        case Ended: return i18nc("process status", "finished");
        default: return i18nc("process status", "unknown");
    }
}

QString KSysGuard::Process::schedulerAsString() const {
    switch( scheduler ) {
        case Fifo: return i18nc("Scheduler", "FIFO");
        case RoundRobin: return i18nc("Scheduler", "Round Robin");
        case Interactive: return i18nc("Scheduler", "Interactive");
        case Batch: return i18nc("Scheduler", "Batch");
        case SchedulerIdle: return i18nc("Scheduler", "Idle");
        default: return QString();
    }
}

void KSysGuard::Process::clear() {
    d->pid = -1;
    parent_pid = -1;
    d->uid = 0;
    d->gid = -1;
    numThreads = 0;
    d->suid = d->euid = d->fsuid = -1;
    d->sgid = d->egid = d->fsgid = -1;
    d->tracerpid = -1;
    d->userTime = 0;
    d->sysTime = 0;
    elapsedTimeMilliSeconds = 0;
    d->startTime = 0;
    userUsage=0;
    sysUsage=0;
    totalUserUsage=0;
    totalSysUsage=0;
    numChildren=0;
    niceLevel=0;
    vmSize=0;
    vmRSS = 0;
    vmURSS = 0;
    vmSizeChange = 0;
    vmRSSChange = 0;
    vmURSSChange = 0;
    pixmapBytes = 0;
    hasManagedGuiWindow = false;
    status=OtherStatus;
    parent = NULL;
    ioPriorityClass = None;
    ioniceLevel = -1;
    scheduler = Other;
    ioCharactersRead = 0;
    ioCharactersWritten = 0;
    ioReadSyscalls = 0;
    ioWriteSyscalls = 0;
    ioCharactersActuallyRead = 0;
    ioCharactersActuallyWritten = 0;
    ioCharactersReadRate = 0;
    ioCharactersWrittenRate = 0;
    ioReadSyscallsRate = 0;
    ioWriteSyscallsRate = 0;
    ioCharactersActuallyReadRate = 0;
    ioCharactersActuallyWrittenRate = 0;

    changes = Process::Nothing;
}

long int KSysGuard::Process::pid() const
{
    return d->pid;
}

QString KSysGuard::Process::login() const
{
    return d->login;
}

qlonglong KSysGuard::Process::uid() const
{
    return d->uid;
}

qlonglong KSysGuard::Process::euid() const
{
    return d->euid;
}

qlonglong KSysGuard::Process::suid() const
{
    return d->suid;
}

qlonglong KSysGuard::Process::fsuid() const
{
    return d->fsuid;
}

qlonglong KSysGuard::Process::gid() const
{
    return d->gid;
}

qlonglong KSysGuard::Process::egid() const
{
    return d->egid;
}

qlonglong KSysGuard::Process::sgid() const
{
    return d->sgid;
}

qlonglong KSysGuard::Process::fsgid() const
{
    return d->fsgid;
}

qlonglong KSysGuard::Process::tracerpid() const
{
    return d->tracerpid;
}

QByteArray KSysGuard::Process::tty() const
{
    return d->tty;
}

qlonglong KSysGuard::Process::userTime() const
{
    return d->userTime;
}

qlonglong KSysGuard::Process::sysTime() const
{
    return d->sysTime;
}

qlonglong KSysGuard::Process::startTime() const
{
    return d->startTime;
}

void KSysGuard::Process::setLogin(QString login)
{
    if(d->login == login) return;
    d->login = login;
    changes |= Process::Login;
}

void KSysGuard::Process::setUid(qlonglong uid)
{
    if(d->uid == uid) return;
    d->uid = uid;
    changes |= Process::Uids;
}

void KSysGuard::Process::setEuid(qlonglong euid)
{
    if(d->euid == euid) return;
    d->euid = euid;
    changes |= Process::Uids;
}

void KSysGuard::Process::setSuid(qlonglong suid) {
    if(d->suid == suid) return;
    d->suid = suid;
    changes |= Process::Uids;
}

void KSysGuard::Process::setFsuid(qlonglong fsuid)
{
    if(d->fsuid == fsuid) return;
    d->fsuid = fsuid;
    changes |= Process::Uids;
}

void KSysGuard::Process::setGid(qlonglong gid)
{
    if(d->gid == gid) return;
    d->gid = gid;
    changes |= Process::Gids;
}

void KSysGuard::Process::setEgid(qlonglong egid)
{
    if(d->egid == egid) return;
    d->egid = egid;
    changes |= Process::Gids;
}

void KSysGuard::Process::setSgid(qlonglong sgid)
{
    if(d->sgid == sgid) return;
    d->sgid = sgid;
    changes |= Process::Gids;
}

void KSysGuard::Process::setFsgid(qlonglong fsgid)
{
    if(d->fsgid == fsgid) return;
    d->fsgid = fsgid;
    changes |= Process::Gids;
}

void KSysGuard::Process::setTracerpid(qlonglong tracerpid)
{
    if(d->tracerpid == tracerpid) return;
    d->tracerpid = tracerpid;
    changes |= Process::Tracerpid;
}

void KSysGuard::Process::setTty(QByteArray tty)
{
    if(d->tty == tty) return;
    d->tty = tty;
    changes |= Process::Tty;
}

void KSysGuard::Process::setUserTime(qlonglong userTime)
{
    d->userTime = userTime;
}

void KSysGuard::Process::setSysTime(qlonglong sysTime)
{
    d->sysTime = sysTime;
}

void KSysGuard::Process::setStartTime(qlonglong startTime)
{
    d->startTime = startTime;
}

void KSysGuard::Process::setUserUsage(int _userUsage) {
    if(userUsage == _userUsage) return;
    userUsage = _userUsage;
    changes |= Process::Usage;
}

void KSysGuard::Process::setSysUsage(int _sysUsage) {
    if(sysUsage == _sysUsage) return;
    sysUsage = _sysUsage;
    changes |= Process::Usage;
}

void KSysGuard::Process::setTotalUserUsage(int _totalUserUsage) {
    if(totalUserUsage == _totalUserUsage) return;
    totalUserUsage = _totalUserUsage;
    changes |= Process::TotalUsage;
}

void KSysGuard::Process::setTotalSysUsage(int _totalSysUsage) {
    if(totalSysUsage == _totalSysUsage) return;
    totalSysUsage = _totalSysUsage;
    changes |= Process::TotalUsage;
}

void KSysGuard::Process::setNiceLevel(int _niceLevel) {
    if(niceLevel == _niceLevel) return;
    niceLevel = _niceLevel;
    changes |= Process::NiceLevels;
}

void KSysGuard::Process::setscheduler(Scheduler _scheduler) {
    if(scheduler == _scheduler) return;
    scheduler = _scheduler;
    changes |= Process::NiceLevels;
}

void KSysGuard::Process::setIoPriorityClass(IoPriorityClass _ioPriorityClass) {
    if(ioPriorityClass == _ioPriorityClass) return;
    ioPriorityClass = _ioPriorityClass;
    changes |= Process::NiceLevels;
}

void KSysGuard::Process::setIoniceLevel(int _ioniceLevel) {
    if(ioniceLevel == _ioniceLevel) return;
    ioniceLevel = _ioniceLevel;
    changes |= Process::NiceLevels;
}

void KSysGuard::Process::setVmSize(qlonglong _vmSize) {
    if(vmSizeChange != 0 || vmSize != 0)
        vmSizeChange = _vmSize - vmSize;
    if(vmSize == _vmSize) return;
    vmSize = _vmSize;
    changes |= Process::VmSize;
}

void KSysGuard::Process::setVmRSS(qlonglong _vmRSS) {
    if(vmRSSChange != 0 || vmRSS != 0)
        vmRSSChange = _vmRSS - vmRSS;
    if(vmRSS == _vmRSS) return;
    vmRSS = _vmRSS;
    changes |= Process::VmRSS;
}

void KSysGuard::Process::setVmURSS(qlonglong _vmURSS) {
    if(vmURSSChange != 0 || vmURSS != 0)
        vmURSSChange = _vmURSS - vmURSS;
    if(vmURSS == _vmURSS) return;
    vmURSS = _vmURSS;
    changes |= Process::VmURSS;
}

void KSysGuard::Process::setName(QString _name) {
    if(name == _name) return;
    name = _name;
    changes |= Process::Name;
}

void KSysGuard::Process::setCommand(QString _command) {
    if(command == _command) return;
    command = _command;
    changes |= Process::Command;
}

void KSysGuard::Process::setStatus(ProcessStatus _status) {
    if(status == _status) return;
    status = _status;
    changes |= Process::Status;
}

void KSysGuard::Process::setIoCharactersRead(qlonglong number) {
    if(number == ioCharactersRead) return;
    ioCharactersRead = number;
    changes |= Process::IO;
}

void KSysGuard::Process::setIoCharactersWritten(qlonglong number) {
    if(number == ioCharactersWritten) return;
    ioCharactersWritten = number;
    changes |= Process::IO;
}

void KSysGuard::Process::setIoReadSyscalls(qlonglong number) {
    if(number == ioReadSyscalls) return;
    ioReadSyscalls = number;
    changes |= Process::IO;
}

void KSysGuard::Process::setIoWriteSyscalls(qlonglong number) {
    if(number == ioWriteSyscalls) return;
    ioWriteSyscalls = number;
    changes |= Process::IO;
}

void KSysGuard::Process::setIoCharactersActuallyRead(qlonglong number) {
    if(number == ioCharactersActuallyRead) return;
    ioCharactersActuallyRead = number;
    changes |= Process::IO;
}

void KSysGuard::Process::setIoCharactersActuallyWritten(qlonglong number) {
    if(number == ioCharactersActuallyWritten) return;
    ioCharactersActuallyWritten = number;
    changes |= Process::IO;
}

void KSysGuard::Process::setIoCharactersReadRate(long number) {
    if(number == ioCharactersReadRate) return;
    ioCharactersReadRate = number;
    changes |= Process::IO;
}

void KSysGuard::Process::setIoCharactersWrittenRate(long number) {
    if(number == ioCharactersWrittenRate) return;
    ioCharactersWrittenRate = number;
    changes |= Process::IO;
}

void KSysGuard::Process::setIoReadSyscallsRate(long number) {
    if(number == ioReadSyscallsRate) return;
    ioReadSyscallsRate = number;
    changes |= Process::IO;
}

void KSysGuard::Process::setIoWriteSyscallsRate(long number) {
    if(number == ioWriteSyscallsRate) return;
    ioWriteSyscallsRate = number;
    changes |= Process::IO;
}

void KSysGuard::Process::setIoCharactersActuallyReadRate(long number) {
    if(number == ioCharactersActuallyReadRate) return;
    ioCharactersActuallyReadRate = number;
    changes |= Process::IO;
}

void KSysGuard::Process::setIoCharactersActuallyWrittenRate(long number) {
    if(number == ioCharactersActuallyWrittenRate) return;
    ioCharactersActuallyWrittenRate = number;
    changes |= Process::IO;
}

void KSysGuard::Process::setNumThreads(int number) {
    if(number == numThreads) return;
    numThreads = number;
    changes |= Process::NumThreads;
}
