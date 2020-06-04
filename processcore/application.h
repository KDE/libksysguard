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

#pragma once

#include <QScopedPointer>
#include <QString>
#include <QVector>

#include <KService>

namespace KSysGuard
{

class Process;

class Q_DECL_EXPORT Application
{
public:
    Application(const QString &processGroup, const KService::Ptr &service);
    virtual ~Application();
    QString id() const;
    KService::Ptr service() const;
    QVector<Process *> processes() const;
private:
    void addProcess(Process *);
    void removeProcess(Process *);
    class Private;
    QScopedPointer<Private> d;
    friend class Applications;
};
}
