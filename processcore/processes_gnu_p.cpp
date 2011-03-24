/*  This file is part of the KDE project
    Copyright (C) 2009 Pino Toscano <pino@kde.org>

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

#include "processes_local_p.h"
#include "process.h"

#ifdef __GNUC__
#warning Totally bogus ProcessesLocal implementation
#endif

namespace KSysGuard
{

class ProcessesLocal::Private
{
public:
    Private()
    {}
    ~Private()
    {}
};


ProcessesLocal::ProcessesLocal()
    : d(0)
{
}

ProcessesLocal::~ProcessesLocal()
{
   delete d;
}

long ProcessesLocal::getParentPid(long pid)
{
    long ppid = -1;
    return ppid;
}

bool ProcessesLocal::updateProcessInfo(long pid, Process *process)
{
    return false;
}

QSet<long> ProcessesLocal::getAllPids()
{
    QSet<long> pids;
    return pids;
}

bool ProcessesLocal::sendSignal(long pid, int sig)
{
    return false;
}

bool ProcessesLocal::setNiceness(long pid, int priority)
{
    return false;
}

bool ProcessesLocal::setScheduler(long pid, int priorityClass, int priority)
{
    return false;
}

bool ProcessesLocal::setIoNiceness(long pid, int priorityClass, int priority)
{
    return false;
}

bool ProcessesLocal::supportsIoNiceness()
{
    return false;
}

long long ProcessesLocal::totalPhysicalMemory()
{
    long long memory = 0;
    return memory;
}

}
