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
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.


*/

#include <klocale.h>
#include <kdebug.h>
#include <QTimer>

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <byteswap.h>
#include <endian.h>
#include <sys/user.h>
#include <ctype.h>

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

#include "DisplayProcessDlg.h"

#include "DisplayProcessDlg.moc"

DisplayProcessDlg::DisplayProcessDlg(QWidget* parent, KSysGuard::Process *process)
	: KDialog( parent )
{
	setObjectName( "Display Process Dialog" );
	setModal( false );
	setCaption( i18n("Monitoring I/O for %1 (%2)", process->pid, process->name) );
	setButtons( Close );
	//enableLinkedHelp( true );
	showButtonSeparator( true );
	
	eight_bit_clean = false;
	follow_forks = true;
	remove_duplicates = false;
	want_to_exit = false;

	mPid = process->pid;

	lastdir = 3;  //an invalid direction, so that the color gets set the first time

	mTextEdit = new KTextEditVT( this );
	setMainWidget( mTextEdit );
	mTextEdit->setReadOnly(true);
	mTextEdit->setParseAnsiEscapeCodes(true);
	mTextEdit->setWhatsThis(i18n("The program '%1' (Pid: %2) is being monitored for input and output through any file descriptor (stdin, stdout, stderr, open files, network connections, etc).  Data being written by the process is shown in red and data being read by the process is shown in blue.", process->name, mPid));
	mTextEdit->document()->setMaximumBlockCount(100);
	mCursor = mTextEdit->textCursor();

	attach(mPid);
	if (attached_pids.isEmpty()) {
		accept();
		return;
	}

	ptrace(PTRACE_SYSCALL, attached_pids[0], 0, 0);

	QTimer *timer = new QTimer(this);
	timer->setSingleShot(false);
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	timer->start(20);
}

DisplayProcessDlg::~DisplayProcessDlg() {
	detach();
}
void DisplayProcessDlg::slotButtonClicked(int)
{
	detach();
	want_to_exit = true; //exit when there's nothing more to parse.  If we exit straight away we risk crashing the process that we are debugging
}

QSize DisplayProcessDlg::sizeHint() const {
	return QSize(600,600);
}

void DisplayProcessDlg::detach() {
	int status;
        foreach(long pid, attached_pids) {
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
	}
	attached_pids.clear();
}

void DisplayProcessDlg::attach(long pid) {
	if (ptrace(PTRACE_ATTACH, pid, 0, 0) == -1) {
		kDebug() << "Failed to attach to process " << pid;
		if(attached_pids.isEmpty())
			accept(); //give up
	} else 
		attached_pids.append(pid);
}

void DisplayProcessDlg::update(bool modified)
{
	static QColor writeColor = QColor(255,0,0);
	static QColor readColor = QColor(0,0,255);

	int status;
	int pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
	if (!WIFSTOPPED(status)) { 
		if(modified)
			mTextEdit->ensureCursorVisible();
		if(want_to_exit)
			accept(); //we can now exit.  Nothing more to process
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
	if (follow_forks && (regs.REG_ORIG_ACCUM == SYS_fork || regs.REG_ORIG_ACCUM == SYS_clone)) {
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
				mTextEdit->moveCursor(QTextCursor::End);
			}
			if(regs.REG_ORIG_ACCUM != lastdir) {
				if(regs.REG_ORIG_ACCUM == SYS_read)
					mTextEdit->setTextColor(readColor);
				else
					mTextEdit->setTextColor(writeColor);
				lastdir = regs.REG_ORIG_ACCUM;
			}
			char c = a&0xff;
			/** Use the KTextEditVT specific function to parse the character 'c' */
			mTextEdit->insertVTChar(QChar(c));
		}
		modified = true;
	}
	ptrace(PTRACE_SYSCALL, pid, 0, 0);
	update(modified);
}
