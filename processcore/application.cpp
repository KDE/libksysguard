/*
    Copyright (c) 2019 David Edmundson <davidedmundson@kde.org>

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

#include "application.h"

#include <QDebug>

#include "process.h"

using namespace KSysGuard;

class Application::Private
{
public:
    Private(const QString &_processGroupId, const KService::Ptr _service)
        : processGroupId(_processGroupId)
        , service(_service)
    {
    }
    const QString processGroupId;
    const KService::Ptr service;
    QVector<Process *> processes;
};

Application::Application(const QString &processGroup, const KService::Ptr &service)
    : d(new Private(processGroup, service))
{
}

Application::~Application()
{
}

QString KSysGuard::Application::id() const
{
    return d->processGroupId;
}

KService::Ptr KSysGuard::Application::service() const
{
    return d->service;
}

QVector<KSysGuard::Process *> KSysGuard::Application::processes() const
{
    return d->processes;
}

void KSysGuard::Application::addProcess(KSysGuard::Process *process)
{
    d->processes.append(process);
}

void KSysGuard::Application::removeProcess(KSysGuard::Process *process)
{
    d->processes.removeOne(process);
}
