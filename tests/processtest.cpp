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


#include <QtTest>
#include <QtCore>

#include <klocale.h>

#include "solidstats/processes.h"
#include "solidstats/process.h"

class testProcess: public QObject
{
    Q_OBJECT
      private slots:
        void testProcesses();
};

void testProcess::testProcesses() {
	QHash<long, Solid::Process *> processes = Solid::Processes::getProcesses();
	foreach( Solid::Process *process, processes)
		kDebug() << process->name << endl;

	QHash<long, Solid::Process *> processes2 = Solid::Processes::getProcesses();
	QCOMPARE(processes, processes2); //Make sure calling it twice gives the same results.  The difference is time is so small that it really shouldn't have changed
}
