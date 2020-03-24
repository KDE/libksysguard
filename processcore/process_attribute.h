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

#include "unit.h"

namespace KSysGuard {

class Process;

class Q_DECL_EXPORT ProcessAttribute : public QObject
{
    Q_OBJECT
public:
    ProcessAttribute(const QString &id, QObject *parent);
    ProcessAttribute(const QString &id, const QString &name, QObject *parent);

    ~ProcessAttribute() override;

    /**
     * A unique non-translatable ID for this attribute. For saving in config files
     */
    QString id() const;

    /**
     * Controls whether we should fetch process attributes
     */
    bool enabled() const;
    void setEnabled(const bool enable);

    /**
     * A translated user facing name for the attribute.
     * e.g "Download Speed"
     */
    QString name() const;
    void setName(const QString &name);

    /**
     * A translated shorter version of the name
     * for use in table column headers for example
     * e.g "D/L"
     * If unset, name is returned
     */
    QString shortName() const;
    void setShortName(const QString &name);

    /**
     * A translated human readable description of this attribute
     */
    QString description() const;
    void setDescription(const QString &description);

    /**
     * The minimum value possible for this sensor
     * (i.e to show a CPU is between 0 and 100)
     * Set min and max to 0 if not relevant
     */
    qreal min() const;
    void setMin(const qreal min);
    /**
     * The maximum value possible for this attribute
     */
    qreal max() const;
    void setMax(const qreal max);

    KSysGuard::Unit unit() const;
    void setUnit(KSysGuard::Unit unit);

    /**
     * A hint to UIs that this sensor would like to be visible by default.
     *
     * Defaults to false.
     */
    bool isVisibleByDefault() const;
    void setVisibleByDefault(bool visible);

    /**
     * The last stored value for a given process
     */
    virtual QVariant data(KSysGuard::Process *process) const;

    /**
     * Updates the stored value for a given process
     * Note stray processes will be automatically expunged
     */
    void setData(KSysGuard::Process *process, const QVariant &value);
    /**
     * Remove an attribute from our local cache
     */
    void clearData(KSysGuard::Process *process);

Q_SIGNALS:
    void dataChanged(KSysGuard::Process *process);
    void enabledChanged(bool enabled);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
