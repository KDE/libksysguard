/*
    Copyright (C) 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

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

#include "Unit.h"

namespace KSysGuard {

/**
 * Tiny helper class to make Formatter usable from QML.
 *
 * An instance of this class will be exposed as a Singleton object to QML. It
 * allows formatting of values from the QML side.
 *
 * This effectively wraps Formatter::formatValue, removing the FormatOptions flag
 * that I couldn't get to work.
 *
 * It is accessible as `Formatter` inside the `org.kde.ksysguard.formatter` package
 * @see Formatter
 */
class FormatterWrapper : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE QString formatValue(const QVariant &value, KSysGuard::Unit unit,
        KSysGuard::MetricPrefix targetPrefix = MetricPrefixAutoAdjust
    );

    Q_INVOKABLE QString formatValueShowNull(const QVariant &value, KSysGuard::Unit unit,
        KSysGuard::MetricPrefix targetPrefix = MetricPrefixAutoAdjust
    );
};

}
