/*
    KSysGuard, the KDE System Guard

        Copyright (C) 2007 Trent Waddington <trent.waddington@gmail.com>
	Copyright (c) 2008 John Tapsell <tapsell@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    aint with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.


*/

#include "../config-ksysguard.h"

#include <klocale.h>
#include <kdebug.h>
#include <QTimer>

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#ifdef HAVE_SYS_PTRACE_H
#include <sys/ptrace.h>
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <endian.h>  //Required to define _BIG_ENDIAN on big endian systems
#include <sys/user.h>
#include <ctype.h>
#if defined(_BIG_ENDIAN) && defined(HAVE_BYTESWAP_H)
//Required for bswap on big endian systems
#include <byteswap.h> 
#endif

#ifdef __i386__
	#define REG_ORIG_ACCUM orig_eax
	#define REG_ACCUM eax
	#define REG_PARAM1 ebx
	#define REG_PARAM2 ecx
	#define REG_PARAM3 edx
#else
#ifdef __amd64__
	#define REG_ORIG_ACCUM orig_rax
	#define REG_ACCUM rax
	#define REG_PARAM1 rdi
	#define REG_PARAM2 rsi
	#define REG_PARAM3 rdx
#else
#if defined(__ppc__) || defined(__powerpc__) || defined(__powerpc64__) || defined(__PPC__) || defined(powerpc)
	#define REG_ORIG_ACCUM gpr[0]
	#define REG_ACCUM gpr[3]
	#define REG_PARAM1 orig_gpr3
	#define REG_PARAM2 gpr[4]
	#define REG_PARAM3 gpr[5]
#ifndef PT_ORIG_R3
	#define PT_ORIG_R3 34
#endif
#endif
#endif
#endif

#include "KMonitorProcessIO.h"

#include "KMonitorProcessIO.moc"

KMonitorProcessIO::KMonitorProcessIO(QWidget* parent, int pid)
	: KTextEditVT( parent ), mPid(pid)
{
	mIncludeChildProcesses = true;
	remove_duplicates = false;

	mUpdateInterval = 20;
	mTimer.setSingleShot(false);
	connect(&mTimer, SIGNAL(timeout()), this, SLOT(update()));

	lastdir = 3;  //an invalid direction, so that the color gets set the first time

	setReadOnly(true);
	setParseAnsiEscapeCodes(true);
	document()->setMaximumBlockCount(100);
	mCursor = textCursor();


	if(pid == -1)
		return;
	attach(mPid);
}

KMonitorProcessIO::~KMonitorProcessIO() {
	detach();
}

int KMonitorProcessIO::updateInterval() const {
	return mUpdateInterval;
}

void KMonitorProcessIO::setUpdateInterval(int msecs) {
	mUpdateInterval = msecs;
	if(mTimer.isActive()) {
		mTimer.stop();
		mTimer.start(msecs); //Start with the new interval time
	}
		
}

void KMonitorProcessIO::detach() {
        foreach(int pid, attached_pids) {
		detach(pid);
	}
}

int KMonitorProcessIO::attachedPid() const {
	return mPid;
}
void KMonitorProcessIO::detach(int pid) {
	int status;
#ifdef HAVE_SYS_PTRACE_H
	if(!ptrace(PTRACE_DETACH, pid, 0, 0)) {
		//successfully detached
	} else if(kill(pid, 0) < 0) {
		if(errno != ESRCH)
			kDebug() << "Something seriously strange when trying to detach.";
	} else if (kill(pid, SIGSTOP) < 0) {
		if (errno != ESRCH)
			kDebug() << "Something seriously strange when trying to detach and then trying to stop the process";
	} else {
		for (;;) {
			if (waitpid(pid, &status, 0) < 0) {
				if (errno != ECHILD)
					kDebug() << "Something seriously strange when trying to detach and waiting for process to stop";
				break;
			}
			if (!WIFSTOPPED(status)) {
				/* Au revoir, mon ami. */
				break;
			}
			if (WSTOPSIG(status) == SIGSTOP) {
				//Okay process is now stopped.  Lets try detaching again.  Silly linux.
				if (ptrace(PTRACE_DETACH,pid, 0, 0) < 0) {
					if (errno != ESRCH)
						kDebug() << "Something seriously strange when trying to detach the second time.";
					/* I died trying. */
				}
				break;
			}
			// we didn't manage to stop the process.  Lets try continuing it and the stopping it
			if (ptrace(PTRACE_CONT, pid, 0, 0) < 0) {
				if (errno != ESRCH)
					kDebug() << "Something seriously strange when trying to detach and continue";
				break;
			}
		  }
	}
#endif
	attached_pids.removeAll(pid);

	if(attached_pids.isEmpty()) {
		mTimer.stop();
	}
}

bool KMonitorProcessIO::reattach() {
	if(mPid == -1)
		return false;
	return attach(mPid);
}

bool KMonitorProcessIO::attach(int pid) {
	if(pid == -1) {
		//Indicates to detach all
		detach();
		return false;
	}
#ifdef HAVE_SYS_PTRACE_H
	if (ptrace(PTRACE_ATTACH, pid, 0, 0) == -1) {
		kDebug() << "Failed to attach to process " << pid;
		if(attached_pids.isEmpty()) {
			mTimer.stop();
			insertHtml(i18n("<br/><i><font color=\"gray\">Failed to attach to process %1</font></i><br/>", pid));
			return false;
		}
	} else {
		if(attached_pids.isEmpty()) {
			//First process added.  Automatically start timer
			ptrace(PTRACE_SYSCALL, pid, 0, 0);
			mTimer.start(mUpdateInterval);
			if(mPid == -1)
				mPid = pid;
		}
		attached_pids.append(pid);
	}
	return true;
#else
	return false;
#endif
}

void KMonitorProcessIO::update(bool modified)
{
#ifdef HAVE_SYS_PTRACE_H
	static QColor writeColor = QColor(255,0,0);
	static QColor readColor = QColor(0,0,255);

	int status;
	int pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
	if (!WIFSTOPPED(status)) { 
		if(modified)
			ensureCursorVisible();
		return;
	}
#if defined(__ppc__) || defined(__powerpc__) || defined(__powerpc64__) || defined(__PPC__) || defined(powerpc)
	struct pt_regs regs;
	regs.gpr[0] = ptrace(PTRACE_PEEKUSER, pid, 4 * PT_R0, 0);
	regs.gpr[3] = ptrace(PTRACE_PEEKUSER, pid, 4 * PT_R3, 0);
	regs.gpr[4] = ptrace(PTRACE_PEEKUSER, pid, 4 * PT_R4, 0);
	regs.gpr[5] = ptrace(PTRACE_PEEKUSER, pid, 4 * PT_R5, 0);
	regs.orig_gpr3 = ptrace(PTRACE_PEEKUSER, pid, 4 * PT_ORIG_R3, 0);
#else
	struct user_regs_struct regs;
	ptrace(PTRACE_GETREGS, pid, 0, &regs);
#endif
	/*unsigned int b = ptrace(PTRACE_PEEKTEXT, pid, regs.eip, 0);*/
	if (mIncludeChildProcesses && (regs.REG_ORIG_ACCUM == SYS_fork || regs.REG_ORIG_ACCUM == SYS_clone)) {
		if (regs.REG_ACCUM > 0)
			attach(regs.REG_ACCUM);					
	}
	if ((regs.REG_ORIG_ACCUM == SYS_read || regs.REG_ORIG_ACCUM == SYS_write) && (regs.REG_PARAM3 == regs.REG_ACCUM)) {
		for (unsigned int i = 0; i < regs.REG_PARAM3; i++) {
			unsigned int a = ptrace(PTRACE_PEEKTEXT, pid, regs.REG_PARAM2 + i, 0);
#ifdef _BIG_ENDIAN
			a = bswap_32(a);
#endif
			if(!modified) {
				//Before we add text or change the color, make sure we are at the end
				moveCursor(QTextCursor::End);
			}
			if(regs.REG_ORIG_ACCUM != lastdir) {
				if(regs.REG_ORIG_ACCUM == SYS_read)
					setTextColor(readColor);
				else
					setTextColor(writeColor);
				lastdir = regs.REG_ORIG_ACCUM;
			}
			char c = a&0xff;
			/** Use the KTextEditVT specific function to parse the character 'c' */
			insertVTChar(QChar(c));
		}
		modified = true;
	}
	ptrace(PTRACE_SYSCALL, pid, 0, 0);
	update(modified);
#endif
}

void KMonitorProcessIO::setIncludeChildProcesses(bool include) {
	mIncludeChildProcesses = include;
}

bool KMonitorProcessIO::includeChildProcesses() const {
	return mIncludeChildProcesses;
}


KMonitorProcessIO::State KMonitorProcessIO::state() const {
	if(attached_pids.isEmpty())
		return Detached;
	if(mTimer.isActive())
		return AttachedRunning;
	return AttachedPaused;
}

void KMonitorProcessIO::pauseProcesses() {
	if(state() == AttachedRunning) {
		mTimer.stop();
	}
}
void KMonitorProcessIO::resumeProcesses() {
	if(state() == AttachedPaused)
		mTimer.start(mUpdateInterval);
}

void KMonitorProcessIO::setState(State new_state) {
	if(new_state == AttachedPaused) pauseProcesses();
	if(new_state == AttachedRunning) resumeProcesses();
	if(new_state == Detached) detach();
}


