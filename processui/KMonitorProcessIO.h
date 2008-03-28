/*
    KSysGuard, the KDE System Guard

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

#ifndef _KMonitorProcessIO_h_
#define _KMonitorProcessIO_h_

#include <QtCore/QTimer>
#include "KTextEditVT.h"
#include <kdialog.h>
#include <kprocess.h>
#include "processes.h"

class KDE_EXPORT KMonitorProcessIO : public KTextEditVT
{
	Q_OBJECT
	Q_PROPERTY( bool includeChildProcesses READ includeChildProcesses WRITE setIncludeChildProcesses )
	Q_PROPERTY( bool running READ running WRITE setRunning )
	Q_PROPERTY( bool updateInterval READ updateInterval WRITE setUpdateInterval )

public:
	KMonitorProcessIO(QWidget* parent, int pid = -1, const QString &name = QString());
	~KMonitorProcessIO();

	/** Set whether to include the output from child processes.  If true, forks and clones will be monitored */
	void setIncludeChildProcesses(bool include);
	/** Whether to include the output from child processes.  If true, forks and clones will be monitored */
	bool includeChildProcesses() const;

	/** Interval to poll for new ptrace input.  Recommended around 20 (milliseconds).  Note that the process
	 *  being monitored cannot do anything if it is waiting for us to update.
	 *  Default is 20 (ms)
	 */
	int updateInterval() const;
	/** Set interval to poll for new ptrace input.  Recommended around 20 (milliseconds).  Note that the process
	 *  being monitored cannot do anything if it is waiting for us to update.
	 *  Default is 20 (ms)
	 */
	void setUpdateInterval(int msecs);

	void detach();
	void detach(int pid);
Q_SIGNALS:
	void finished();
private Q_SLOTS:
	/** Read in the next bit of data and display it.  This should be called very frequently. */
	void update(bool modified=false);
	bool running() const;
	void setRunning(bool run);

private:
	KProcess mIOProcess;
	KTextEditVT *mTextEdit;
	QTimer mTimer;
	long mPid;
	QList<long> attached_pids;

	int mUpdateInterval;
	bool mIncludeChildProcesses;
	bool remove_duplicates;
	void attach(long);

	unsigned int lastdir;
	QTextCursor mCursor;
};

#endif

