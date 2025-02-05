/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "application_data_model.h"

#include <KUser>

#include <QDebug>

#include "cgroup.h"

using namespace KSysGuard;
using namespace Qt::StringLiterals;

ApplicationDataModel::ApplicationDataModel(QObject *parent)
    : CGroupDataModel(QStringLiteral("/user.slice/user-%1.slice/user@%1.service").arg(KUserId::currentEffectiveUserId().toString()), parent)
{
}

ApplicationDataModel::~ApplicationDataModel() = default;

QVariantMap KSysGuard::ApplicationDataModel::cgroupMapping() const
{
    return m_cgroupMapping;
}

void KSysGuard::ApplicationDataModel::setCGroupMapping(const QVariantMap &newMapping)
{
    if (newMapping == m_cgroupMapping) {
        return;
    }

    m_cgroupMapping = newMapping;

    m_mappedCgroups.clear();
    std::ranges::transform(m_cgroupMapping.asKeyValueRange(), std::inserter(m_mappedCgroups, m_mappedCgroups.begin()), [](auto entry) {
        return entry.second.toString();
    });

    m_cgroupMappingCache.clear();

    Q_EMIT cgroupMappingChanged();
}

bool ApplicationDataModel::filterAcceptsCGroup(CGroup *cgroup)
{
    if (!isCgroupRelevant(cgroup)) {
        return false;
    }

    auto itr = m_applications.find(applicationId(cgroup));
    if (itr == m_applications.end()) {
        return false;
    }

    // Only display an entry for the first item in an application's cgroup set.
    if (itr.value().cgroups.empty() || cgroup != itr.value().cgroups.first()) {
        return false;
    }

    return true;
}

void ApplicationDataModel::cgroupAdded(CGroup *cgroup)
{
    if (!isCgroupRelevant(cgroup)) {
        return;
    }

    auto appId = applicationId(cgroup);

    auto itr = m_applications.find(appId);
    if (itr == m_applications.end()) {
        itr = m_applications.emplace(appId);
        itr.value().applicationId = appId;
    }

    itr.value().cgroups.append(cgroup);
}

void ApplicationDataModel::cgroupRemoved(CGroup *cgroup)
{
    auto itr = m_applications.find(applicationId(cgroup));
    if (itr == m_applications.end()) {
        return;
    }

    auto &application = itr.value();

    application.cgroups.removeOne(cgroup);
    m_cgroupMappingCache.remove(cgroup->id());

    if (itr.value().cgroups.empty()) {
        m_applications.erase(itr);
    }
}

QList<Process *> ApplicationDataModel::processesFor(CGroup *cgroup) const
{
    auto itr = m_applications.find(applicationId(cgroup));
    if (itr == m_applications.end() || itr.value().cgroups.isEmpty()) {
        return {};
    }

    QList<Process *> result;
    for (auto cgroup : std::as_const(itr.value().cgroups)) {
        result.append(CGroupDataModel::processesFor(cgroup));
    }

    return result;
}

bool ApplicationDataModel::isCgroupRelevant(CGroup *cgroup)
{
    if (!CGroupDataModel::filterAcceptsCGroup(cgroup)) {
        return false;
    }

    auto cgroupId = cgroup->id();
    auto appId = applicationId(cgroup);

    if (m_mappedCgroups.contains(appId)) {
        return true;
    }

    if (!cgroupId.contains(QLatin1String("/app-")) && !(cgroup->id().contains(QLatin1String("/flatpak")) && cgroup->id().endsWith(QLatin1String("scope")))) {
        return false;
    }

    // Certain DBus launched things will end up creating a CGroup that looks like
    // dbus-:1.2-org.telegram.desktop@0.service . Since these don't actually contain
    // anything relevant, just filter them out as they end up just cluttering the
    // list.
    if (appId.startsWith(u"dbus-:"_s)) {
        return false;
    }

    return true;
}

QString ApplicationDataModel::applicationId(CGroup *cgroup) const
{
    auto cgroupId = cgroup->id();
    if (m_cgroupMappingCache.contains(cgroupId)) {
        return m_cgroupMappingCache.value(cgroupId);
    }

    QString appId;

    for (auto [cgroup, mappedName] : m_cgroupMapping.asKeyValueRange()) {
        if (cgroupId.contains(cgroup, Qt::CaseSensitivity::CaseInsensitive)) {
            appId = mappedName.toString();
            break;
        }
    }

    if (appId.isEmpty()) {
        appId = cgroup->service()->storageId();
        if (appId.isEmpty()) {
            appId = cgroup->service()->name();
        }
    }

    m_cgroupMappingCache.insert(cgroupId, appId);

    return appId;
}
