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
#include <QVariant>

namespace KSysGuard {

class Processes;
class Process;
class ProcessAttribute;

/**
 * Base class for a process plugin data
 * Plugins provide a list of additional attributes, which in turn have data about a given process
 */
class Q_DECL_EXPORT ProcessDataProvider : public QObject
{
    Q_OBJECT

public:
    ProcessDataProvider(QObject *parent, const QVariantList &args);
    ~ProcessDataProvider() override;

    /**
     * Accessors for process information matching
     */
    KSysGuard::Processes* processes() const;

    /**
     * Returns a new process object for a given PID
     * This will update the process list if this PID does not exist yet
     * This may return a null pointer
     */
    KSysGuard::Process * getProcess(long pid);

    /**
     * A list of all process attributes provided by this plugin
     * It is expected to remain constant through the lifespan of this class
     */
    QVector<ProcessAttribute *> attributes() const;

    /**
     * Called when processes should be updated if manually polled
     * Plugins can however update at any time if enabled
     */
    virtual void update()
    {}

    /**
     * True when at least one attribute from this plugin is subscribed
     */
    bool enabled() const;

    virtual void handleEnabledChanged(bool enabled)
    {
        Q_UNUSED(enabled)
    }

    // for any future compatibility
    virtual void virtual_hook(int id, void *data)
    {
        Q_UNUSED(id)
        Q_UNUSED(data)
    }

protected:
    /**
     * Register a new process attribute
     * Process attributes should be created in the plugin constuctor and must live for the duration the plugin
     */
    void addProcessAttribute(ProcessAttribute *attribute);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
