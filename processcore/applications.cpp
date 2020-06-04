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

#include "applications.h"
#include "application.h"

#include <QHash>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDebug>

#include <KService>

#include "process.h"
#include "processes.h"

namespace KSysGuard
{
class Applications::Private
{
public:
    Private(Applications *_q);
    KService::Ptr serviceFromAppId(const QString &appId) const;
    QString appIdFromProcessGroup(QString processGroup) const;
    void handleAddProcess(Process *process);

    Applications *q;
    QHash<QString, Application *> appMap;
    QList<Application *> apps;
    //hack round KProcess API so we don't create things on beginCreate.
    //Maybe we should refactor signals
    Process *pendingProcess;
    QRegularExpression appIdFromProcessGroupPattern;

};
}

using namespace KSysGuard;

Applications::Applications(KSysGuard::Processes *processList, QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    d->appIdFromProcessGroupPattern.setPattern(QStringLiteral("[apps|app|flatpak]-([^-]+)-.*"));

    //Batch processes into Application objects
    connect(processList, &Processes::beginAddProcess, this, [this](Process *process) {
        d->pendingProcess = process;
    });

    connect(processList, &Processes::endAddProcess, this, [this]() {
        KSysGuard::Process *process = d->pendingProcess;
        d->pendingProcess = nullptr;
        if (!process) {
            return;
        }
        d->handleAddProcess(process);
    });

    connect(processList, &Processes::beginRemoveProcess, this, [this](Process *process) {
        const QString processGroup = process->cGroup();
        const auto it = d->appMap.constFind(processGroup);
        if (it == d->appMap.constEnd()) {
            return;
        }
        const auto app = it.value();
        app->removeProcess(process);
        if (app->processes().isEmpty()) {
            emit beginApplicationRemoved(app);
            d->apps.removeOne(app);
            d->appMap.erase(it);
            delete app;
            emit endApplicationRemoved();
        } else {
            emit applicationChanged(it.value());
        }
    });

    connect(processList, &Processes::processChanged, this, [this](Process *process) {
        const QString processGroup = process->cGroup();
        auto app = d->appMap.value(processGroup);
        if (app) {
            emit applicationDataChanged(app);
        }
    });

    for (auto process: processList->getAllProcesses()) {
        d->handleAddProcess(process);
    }
}

Applications::~Applications()
{
    qDeleteAll(d->appMap);
}

Application *KSysGuard::Applications::getApplication(const QString &id) const
{
    return d->appMap.value(id);
}

QList<Application *> Applications::getAllApplications() const
{
    return d->apps;
}

Applications::Private::Private(Applications *_q)
    :q(_q)
{}

QString Applications::Private::appIdFromProcessGroup(QString processGroup) const
{
    const int lastSlash = processGroup.lastIndexOf(QLatin1Char('/'));

    if (lastSlash != -1) {
        processGroup = processGroup.mid(lastSlash);
    }

    const QRegularExpressionMatch &appId = appIdFromProcessGroupPattern.match(processGroup);

    if (!appId.isValid() || !appId.hasMatch()) {
        return QString();
    }

    return appId.captured(1);
}

KService::Ptr Applications::Private::serviceFromAppId(const QString &appId) const
{
    if (appId.isEmpty()) {
        return KService::Ptr();
    }

    KService::Ptr service = KService::serviceByDesktopName(appId);

    if (!service) {
        service = KService::serviceByMenuId(appId + QStringLiteral(".desktop"));
    }

    return service;
}

void Applications::Private::handleAddProcess (Process *process)
{
    const QString processGroup = process->cGroup();
    const auto it = appMap.constFind(processGroup);
    if (it == appMap.constEnd()) {
        const QString appId = appIdFromProcessGroup(processGroup);
        KService::Ptr service = serviceFromAppId(appId);
        if (!service || !service->isValid()) {
            return;
        }
        auto app = new Application(processGroup, service);
        app->addProcess(process);
        emit q->beginApplicationAdded(app);
        appMap.insert(processGroup, app);
        apps.append(app);
        emit q->endApplicationAdded(app);
    } else {
        it.value()->addProcess(process);
        emit q->applicationChanged(it.value());
    }
};
