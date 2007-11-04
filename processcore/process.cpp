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

#include "process.h"


KSysGuard::Process::Process() { 
	clear();
}
KSysGuard::Process::Process(long long _pid, long long _ppid, Process *_parent)  {
	clear();
	pid = _pid;
	parent_pid = _ppid;
	parent = _parent;
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
		default: return i18nc("process status", "unknown");
	}
}

QString KSysGuard::Process::schedulerAsString() const { 
	switch( scheduler ) { 
		case Fifo: return i18nc("Scheduler", "FIFO");
		case RoundRobin: return i18nc("Scheduler", "Round Robin");
		case Batch: return i18nc("Scheduler", "Batch");
		default: return QString();
	}
}

void KSysGuard::Process::clear() {
	pid = 0; 
	parent_pid = 0; 
	uid = 0; 
	gid = -1; 
	suid = euid = fsuid = -1;
	sgid = egid = fsgid = -1;
	tracerpid = 0; 
	userTime = -1; 
	sysTime = -1; 
	userUsage=0; 
	sysUsage=0; 
	totalUserUsage=0; 
	totalSysUsage=0; 
	numChildren=0; 
	niceLevel=0; 
	vmSize=0; 
	vmRSS = 0; 
	vmURSS = 0; 
	status=OtherStatus;
	parent = NULL;
	ioPriorityClass = None;
	ioniceLevel = -1;
	scheduler = Other;
}

