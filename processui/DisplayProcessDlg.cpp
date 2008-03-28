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

#include "KMonitorProcessIO.h"
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

	mTextEdit = new KMonitorProcessIO( this, process->pid, process->name );
	setMainWidget( mTextEdit );
}

DisplayProcessDlg::~DisplayProcessDlg() {
	mTextEdit->detach();
}
void DisplayProcessDlg::slotButtonClicked(int)
{
	mTextEdit->detach();
	accept();
}

QSize DisplayProcessDlg::sizeHint() const {
	return QSize(600,600);
}

