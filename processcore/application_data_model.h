/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "cgroup_data_model.h"

#include "processcore_export.h"

namespace KSysGuard
{
class Process;

/**
 * This is a subclass of CGroupDataModel that combines separate CGroups with
 * matching application IDs into a single row.
 */
class PROCESSCORE_EXPORT ApplicationDataModel : public CGroupDataModel
{
    Q_OBJECT
public:
    ApplicationDataModel(QObject *parent = nullptr);
    ~ApplicationDataModel() override;

    /*!
     * A map containing (parts of) CGroup IDs and application IDs to use for those CGroups.
     *
     * Each key in the map is expected to be either a full CGroup ID or a string
     * that matches a part of a CGroup ID. When we try to lookup the application
     * ID for a CGroup that matches one of the keys, we return the provided
     * application ID instead of trying to determine the application ID based on
     * the CGroup name.
     *
     * Since the model combines all CGroups that match a certain application ID
     * into the same row, if you use the same application ID for multiple
     * entries any CGroups matching those entries will be combined into the same
     * application row.
     *
     * Note that the mapped application will always be included as a row in the
     * model. Also note that the mapped application ID does not need to be an
     * actual valid application ID.
     */
    Q_PROPERTY(QVariantMap cgroupMapping READ cgroupMapping WRITE setCGroupMapping NOTIFY cgroupMappingChanged)
    QVariantMap cgroupMapping() const;
    void setCGroupMapping(const QVariantMap &newMapping);
    Q_SIGNAL void cgroupMappingChanged();

protected:
    bool filterAcceptsCGroup(CGroup *cgroup) override;
    void cgroupAdded(CGroup *cgroup) override;
    void cgroupRemoved(CGroup *cgroup) override;
    QList<Process *> processesFor(CGroup *cgroup) const override;

private:
    struct Application {
        QString applicationId;
        QList<CGroup *> cgroups;
    };

    bool isCgroupRelevant(CGroup *cgroup);
    QString applicationId(CGroup *cgroup) const;

    QHash<QString, Application> m_applications;

    QVariantMap m_cgroupMapping;
    // Helper to speed up lookup of mapped CGroups. Contains the unique values
    // of m_groupMapping.
    QSet<QString> m_mappedCgroups;
    // A cache of full CGroup name to application ID. This avoids having to
    // re-run the mapping every time we want an application ID for a cgroup.
    mutable QHash<QString, QString> m_cgroupMappingCache;
};

}
