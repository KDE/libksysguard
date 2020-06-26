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

#include "cgroup.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringView>

#include "process.h"

using namespace KSysGuard;

class KSysGuard::CGroupPrivate
{
public:
    CGroupPrivate(const QString &_processGroupId)
        : processGroupId(_processGroupId)
        , service(serviceFromAppId(_processGroupId))
    {
    }
    const QString processGroupId;
    const KService::Ptr service;
    QVector<Process *> processes;
    static KService::Ptr serviceFromAppId(const QString &appId);

    static QRegularExpression s_appIdFromProcessGroupPattern;
    static QString unescapeName(const QString &cgroupId);
    QVector<Process *> procs;
};

class CGroupSystemInformation
{
public:
    CGroupSystemInformation();
    QString sysGgroupRoot;
};

Q_GLOBAL_STATIC(CGroupSystemInformation, s_cGroupSystemInformation)

// Flatpak's are currently in a cgroup, but they don't follow the specification
// this has been fixed, but this provides some compatability till that lands
// app vs apps exists because the spec changed.
QRegularExpression CGroupPrivate::s_appIdFromProcessGroupPattern(QStringLiteral("[apps|app|flatpak]-([^-]+)-.*"));

CGroup::CGroup(const QString &id)
    : d(new CGroupPrivate(id))
{
}

CGroup::~CGroup()
{
}

QString KSysGuard::CGroup::id() const
{
    return d->processGroupId;
}

KService::Ptr KSysGuard::CGroup::service() const
{
    return d->service;
}

QVector<Process *> CGroup::processes() const
{
    return d->procs;
}

void CGroup::setProcesses(QVector<Process *> procs)
{
    d->procs = procs;
}

QVector<pid_t> KSysGuard::CGroup::getPids() const
{
    const QString pidFilePath = cgroupSysBasePath() + d->processGroupId + QLatin1String("/cgroup.procs");
    QFile pidFile(pidFilePath);
    pidFile.open(QFile::ReadOnly | QIODevice::Text);
    QTextStream stream(&pidFile);

    QVector<pid_t> procs;
    QString line = stream.readLine();
    while (!line.isNull()) {
        procs.append(line.toLong());
        line = stream.readLine();
    }

    return procs;
}

QString CGroupPrivate::unescapeName(const QString &name) {
    // strings are escaped in the form of \xZZ where ZZ is a two digits in hex representing an ascii code
    QString rc = name;
    while (true) {
        int escapeCharIndex = rc.indexOf(QLatin1Char('\\'));
        if (escapeCharIndex < 0) {
            break;
        }
        const QStringView sequence = rc.mid(escapeCharIndex, 4);
        if (sequence.length() != 4 || sequence.at(1) != QLatin1Char('x')) {
            qWarning() << "Badly formed cgroup name" << name;
            return name;
        }
        bool ok;
        int character = sequence.mid(2).toString().toInt(&ok, 16);
        if (ok) {
            rc.replace(escapeCharIndex, 4, QLatin1Char(character));
        }
    }
    return rc;
}



KService::Ptr CGroupPrivate::serviceFromAppId(const QString &processGroup)
{
    const int lastSlash = processGroup.lastIndexOf(QLatin1Char('/'));

    QString serviceName = processGroup;
    if (lastSlash != -1) {
        serviceName = processGroup.mid(lastSlash + 1);
    }
    serviceName = unescapeName(serviceName);

    const QRegularExpressionMatch &appIdMatch = s_appIdFromProcessGroupPattern.match(serviceName);

    if (!appIdMatch.isValid() || !appIdMatch.hasMatch()) {
        // create a transient service object just to have a sensible name
        return KService::Ptr(new KService(serviceName, QString(), QString()));
    }

    const QString appId = appIdMatch.captured(1);

    KService::Ptr service = KService::serviceByMenuId(appId + QStringLiteral(".desktop"));

    if (!service) {
        service = new KService(appId, QString(), QString());
    }

    return service;
}

QString CGroup::cgroupSysBasePath()
{
    return s_cGroupSystemInformation->sysGgroupRoot;
}

CGroupSystemInformation::CGroupSystemInformation()
{
    QDir base(QStringLiteral("/sys/fs/cgroup"));
    if (base.exists(QLatin1String("unified"))) {
        sysGgroupRoot = base.absoluteFilePath(QStringLiteral("unified"));
        return;
    }
    if (base.exists()) {
        sysGgroupRoot = base.absolutePath();
    }
}


