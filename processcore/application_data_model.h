/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "cgroup_data_model.h"

namespace KSysGuard
{
class Q_DECL_EXPORT ApplicationDataModel : public CGroupDataModel
{
    Q_OBJECT
public:
    ApplicationDataModel(QObject *parent = nullptr);
    bool filterAcceptsCGroup(const QString &id) override;
};

}
