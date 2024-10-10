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
};

}
