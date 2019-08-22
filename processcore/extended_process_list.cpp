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
#include "extended_process_list.h"

#include <KPluginLoader>
#include <KPluginFactory>
#include <KPluginMetaData>

#include "process_data_provider.h"
#include "process_attribute.h"
#include "processcore_debug.h"

using namespace  KSysGuard;

class ExtendedProcesses::Private
{
public:
    Private(ExtendedProcesses *q);
    void loadPlugins();

    ExtendedProcesses *q;
    QVector<ProcessDataProvider *> m_providers;
};

ExtendedProcesses::Private::Private(ExtendedProcesses *_q)
    :q(_q)
{}

ExtendedProcesses::ExtendedProcesses(QObject *parent):
    Processes(QString(), parent),
    d(new Private(this))
{
    d->loadPlugins();

    connect(this, &KSysGuard::Processes::beginRemoveProcess, this, [this](KSysGuard::Process *process) {
        const auto attrs = attributes();
        for (auto a: attrs) {
            a->clearData(process);
        }
    });

    connect(this, &KSysGuard::Processes::updated, this, [this]() {
        for (auto p: qAsConst(d->m_providers)) {
            if (p->enabled()) {
                p->update();
            }
        }
    });
}

ExtendedProcesses::~ExtendedProcesses()
{
}

QVector<ProcessAttribute *> ExtendedProcesses::attributes() const
{
    QVector<ProcessAttribute *> rc;
    for (auto p: qAsConst(d->m_providers)) {
        rc << p->attributes();
    }
    return rc;
}

void ExtendedProcesses::Private::loadPlugins()
{
    //instantiate all plugins
    const QVector<KPluginMetaData> listMetaData = KPluginLoader::findPlugins(QStringLiteral("ksysguard/process"));
    for (const auto &pluginMetaData: listMetaData) {
        qCDebug(LIBKSYSGUARD_PROCESSCORE) << "loading plugin" << pluginMetaData.name();
        auto factory = qobject_cast<KPluginFactory*>(pluginMetaData.instantiate());
        if (!factory) {
            qCCritical(LIBKSYSGUARD_PROCESSCORE) << "failed to load plugin factory" << pluginMetaData.name();
            continue;
        }
        ProcessDataProvider *provider = factory->create<ProcessDataProvider>(q);
        if (!provider) {
            qCCritical(LIBKSYSGUARD_PROCESSCORE) << "failed to instantiate ProcessDataProvider" << pluginMetaData.name();
            continue;
        }
        m_providers << provider;
    }
}


