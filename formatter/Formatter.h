/*
    Copyright (C) 2019 Vlad Zagorodniy <vladzzag@gmail.com>

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

// Own
#include "formatter_export.h"
#include "Unit.h"

// Qt
#include <QString>
#include <QVariant>

class KLocalizedString;

namespace KSysGuard
{

/**
 * This enum type is used to specify format options.
 */
enum FormatOption {
    FormatOptionNone = 0,
    FormatOptionAgo = 1 << 0,
    FormatOptionShowNull = 1 << 1,
};
Q_DECLARE_FLAGS(FormatOptions, FormatOption)

class FORMATTER_EXPORT Formatter
{
public:
    /**
     * Returns the scale factor suitable for display.
     *
     * @param value The maximum output value.
     * @param unit The unit of the value.
     * @param targetPrefix Preferred metric prefix.
     */
    static qreal scaleDownFactor(const QVariant &value, Unit unit,
        MetricPrefix targetPrefix = MetricPrefixAutoAdjust);

    /**
     * Returns localized string that is suitable for display.
     *
     * @param value The maximum output value.
     * @param unit The unit of the value.
     * @param targetPrefix Preferred metric prefix.
     */
    static KLocalizedString localizedString(const QVariant &value, Unit unit,
        MetricPrefix targetPrefix = MetricPrefixAutoAdjust);

    /**
     * Converts @p value to the appropriate displayable string.
     *
     * The returned string is localized.
     *
     * @param value The value to be converted.
     * @param unit The unit of the value.
     * @param targetPrefix Preferred metric prefix.
     * @param options
     */
    static QString formatValue(const QVariant &value, Unit unit,
        MetricPrefix targetPrefix = MetricPrefixAutoAdjust,
        FormatOptions options = FormatOptionNone);

    /**
     * Returns a symbol that corresponds to the given @p unit.
     *
     * The returned unit symbol is localized.
     */
    static QString symbol(Unit unit);
};

} // namespace KSysGuard

Q_DECLARE_OPERATORS_FOR_FLAGS(KSysGuard::FormatOptions)
