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

#include "process_data_provider.h"
#include "processes.h"
#include "process_attribute.h"

using namespace KSysGuard;

class Q_DECL_HIDDEN KSysGuard::ProcessDataProvider::Private
{
public:
    KSysGuard::Processes *m_processes;
    QVector<ProcessAttribute *> m_attributes;
    bool m_enabled = false;
};

ProcessDataProvider::ProcessDataProvider(QObject *parent, const QVariantList &args)
    : QObject(parent)
    , d(new Private)
{
    //cast is needed to allow us to use KPluginFactory, but not have null pointers during subclass construction
    auto procList = qobject_cast<KSysGuard::Processes *>(parent);
    Q_ASSERT(procList);
    d->m_processes = procList;

    Q_UNUSED(args)
}

ProcessDataProvider::~ProcessDataProvider()
{
}

KSysGuard::Processes *ProcessDataProvider::processes() const
{
    return d->m_processes;
}

KSysGuard::Process *ProcessDataProvider::getProcess(long pid)
{
    auto process = d->m_processes->getProcess(pid);
    if (!process) {
        processes()->updateOrAddProcess(pid);
    }
    process = processes()->getProcess(pid);
    return process;
}

QVector<ProcessAttribute *> ProcessDataProvider::attributes() const
{
    return d->m_attributes;
}

bool ProcessDataProvider::enabled() const
{
    return d->m_enabled;
}

void ProcessDataProvider::addProcessAttribute(ProcessAttribute *attribute)
{
    d->m_attributes << attribute;
    connect(attribute, &ProcessAttribute::enabledChanged, this, [this](bool enabled) {
        if (enabled == d->m_enabled) {
            return;
        }
        bool wasEnabled = d->m_enabled;

        d->m_enabled = std::any_of(d->m_attributes.constBegin(), d->m_attributes.constEnd(), [](ProcessAttribute *p) {
            return p->enabled();
        });

        if (d->m_enabled != wasEnabled) {
            handleEnabledChanged(d->m_enabled);
        }
    });
}
