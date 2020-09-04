/*
    Copyright (c) 2019 David Edmundson <davidedmundson@kde.org>
    Copyright (c) 2020 David Redondo <kde@david-redondo.de>

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
#include <QThreadPool>

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
    QVector<pid_t> pids;
    std::mutex pidsLock;

    static KService::Ptr serviceFromAppId(const QString &appId);

    static QRegularExpression s_appIdFromProcessGroupPattern;
    static QString unescapeName(const QString &cgroupId);

    class ReadPidsRunnable;
    ReadPidsRunnable *readPids = nullptr;
};

class CGroupPrivate::ReadPidsRunnable : public QRunnable
{
public:
    ReadPidsRunnable(CGroupPrivate *cgroup, QPointer<QObject> context, const QString &path, std::function<void()> callback)
        : m_cgroupPrivate(cgroup)
        , m_context(context)
        , m_path(path)
        , m_callback(callback)
    {
    }

    void run() override
    {
        QFile pidFile(m_path);
        pidFile.open(QFile::ReadOnly | QIODevice::Text);
        QTextStream stream(&pidFile);

        QVector<pid_t> pids;
        QString line = stream.readLine();
        while (!line.isNull()) {
            pids.append(line.toLong());
            line = stream.readLine();
        }
        m_cgroupPrivate->pids = pids;

        // Ensure we call the callback on the thread the context object lives on.
        if (m_context) {
            QMetaObject::invokeMethod(m_context, m_callback);
        }

        std::lock_guard<std::mutex> lock{m_lock};
        m_finished = true;
        m_cgroupPrivate->readPids = nullptr;
        m_condition.notify_all();
    }

    void wait()
    {
        std::unique_lock<std::mutex> lock{m_lock};
        m_condition.wait(lock, [this]() { return m_finished; });
    }

private:
    CGroupPrivate *m_cgroupPrivate;
    QPointer<QObject> m_context;
    QString m_path;
    std::function<void()> m_callback;

    std::mutex m_lock;
    std::condition_variable m_condition;
    bool m_finished = false;
};

class CGroupSystemInformation
{
public:
    CGroupSystemInformation();
    QString sysGgroupRoot;
};

Q_GLOBAL_STATIC(CGroupSystemInformation, s_cGroupSystemInformation)

// The spec says that the two following schemes are allowed
// - app[-<launcher>]-<ApplicationID>-<RANDOM>.scope
// - app[-<launcher>]-<ApplicationID>[@<RANDOM>].service
// Flatpak's are currently in a cgroup, but they don't follow the specification
// this has been fixed, but this provides some compatability till that lands
// app vs apps exists because the spec changed.
QRegularExpression CGroupPrivate::s_appIdFromProcessGroupPattern(QStringLiteral("[app|apps|flatpak]-(?:[^-]*-)?([^-]+(?=-.*\\.scope)|[^@]+(?=(?:@.*)?\\.service))"));

CGroup::CGroup(const QString &id)
    : d(new CGroupPrivate(id))
{
}

CGroup::~CGroup()
{
    if (d->readPids) {
        d->readPids->wait();
    }
}

QString KSysGuard::CGroup::id() const
{
    return d->processGroupId;
}

KService::Ptr KSysGuard::CGroup::service() const
{
    return d->service;
}

QVector<pid_t> CGroup::pids() const
{
    if (d->readPids) {
        d->readPids->wait();
    }
    return d->pids;
}

void CGroup::requestPids(QPointer<QObject> context, std::function<void()> callback)
{
    if (d->readPids) {
        d->readPids->wait();
    }

    QString path = cgroupSysBasePath() + d->processGroupId + QLatin1String("/cgroup.procs");
    d->readPids = new CGroupPrivate::ReadPidsRunnable(d.get(), context, path, callback);
    QThreadPool::globalInstance()->start(d->readPids);
}

QString CGroupPrivate::unescapeName(const QString &name) {
    // strings are escaped in the form of \xZZ where ZZ is a two digits in hex representing an ascii code
    QString rc = name;
    while (true) {
        int escapeCharIndex = rc.indexOf(QLatin1Char('\\'));
        if (escapeCharIndex < 0) {
            break;
        }
        const QStringRef sequence = rc.midRef(escapeCharIndex, 4);
        if (sequence.length() != 4 || sequence.at(1) != QLatin1Char('x')) {
            qWarning() << "Badly formed cgroup name" << name;
            return name;
        }
        bool ok;
        int character = sequence.mid(2).toInt(&ok, 16);
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


