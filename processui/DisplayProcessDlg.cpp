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

#include <klocale.h>
#include <kdebug.h>

#include "DisplayProcessDlg.h"

#include "DisplayProcessDlg.moc"

DisplayProcessDlg::DisplayProcessDlg(QWidget* parent, const QString & processname, const QStringList & args, KSysGuard::Process *process)
	: KDialog( parent )
{
	setObjectName( "Display Process Dialog" );
	setModal( false );
	setCaption( i18n("Monitoring IO for %1 (%2)", process->pid, process->name) );
	setButtons( Close );
	showButtonSeparator( true );

	mIOProcess << processname << args;
	connect(&mIOProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
	connect(&mIOProcess, SIGNAL(finished( int, QProcess::ExitStatus)), this, SLOT(finished( int, QProcess::ExitStatus)));

	connect(&mIOProcess, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
	connect(&mIOProcess, SIGNAL(readyReadStandardOutput ()), this, SLOT(readyReadStandardOutput ()));
	connect(&mIOProcess, SIGNAL(started ()), this, SLOT(started ()));
	connect(&mIOProcess, SIGNAL(stateChanged ( QProcess::ProcessState)), this, SLOT(stateChanged ( QProcess::ProcessState)));
	mIOProcess.setOutputChannelMode( KProcess::MergedChannels);
	mIOProcess.start();
	mTextEdit = new QTextEdit( this );
	setMainWidget( mTextEdit );
}
DisplayProcessDlg::~DisplayProcessDlg() {
}
void DisplayProcessDlg::slotButtonClicked(int)
{
	mIOProcess.kill();
}
void DisplayProcessDlg::error ( QProcess::ProcessError error ) {
	accept();
}
void DisplayProcessDlg::finished ( int exitCode, QProcess::ExitStatus exitStatus ) {
	accept();
}
void DisplayProcessDlg::readyReadStandardError() {
}
void DisplayProcessDlg::readyReadStandardOutput() {
	mTextEdit->append(QString::fromUtf8(mIOProcess.readAllStandardOutput()));
}
void DisplayProcessDlg::started () {
}
void DisplayProcessDlg::stateChanged ( QProcess::ProcessState newState ) {
}

QSize DisplayProcessDlg::sizeHint() const {
	return QSize(600,600);
}
