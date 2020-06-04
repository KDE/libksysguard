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

#include <QObject>

namespace KSysGuard
{
class Application;
class Processes;

class Q_DECL_EXPORT Applications : public QObject
{
    Q_OBJECT
public:
    Applications(Processes *processList, QObject *parent = nullptr);
    ~Applications() override;

    Application *getApplication(const QString &id) const;
    QList<Application *> getAllApplications() const;

Q_SIGNALS:
    /**
    * Emitted when a process has been added or removed from an application
    */
    void applicationChanged(Application *);
    /**
     * Emitted when the process data within an application has changed
     */
    void applicationDataChanged(Application *);
    /**
     * A new process has been added with a given application ID
     */
    void beginApplicationAdded(Application *);

    void endApplicationAdded(Application *);
    /**
     * No processes now have the given application ID
     */
    void beginApplicationRemoved(Application *);
    void endApplicationRemoved();

private:
    class Private;
    QScopedPointer<Private> d;
};
}
