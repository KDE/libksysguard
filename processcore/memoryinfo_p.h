/*
 *  SPDX-FileCopyrightText: 2024 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <chrono>

#include <QObject>

namespace KSysGuard
{

struct MemoryFields {
    enum Field {
        Rss,
        Shared,
        Private,
        Swap,
        Other,
    };

    std::chrono::system_clock::time_point lastUpdate;

    union {
        struct {
            qlonglong rss = -1;
            qlonglong shared = -1;
            qlonglong priv = -1;
            qlonglong swap = -1;
            // To be used depending on context.
            qlonglong other = -1;
        };
        qlonglong fields[5];
    };
};

/*
 * A struct to keep track of memory information.
 *
 * There are a number of values we want to track for memory usage:
 *
 * - RSS, resident set size, the amount of memory used by a process including
 *        any shared memory.
 * - PSS, proportional set size, the amount of memory used by a process including
 *        the proportion of shared memory that a specif process contributes.
 * - Shared, the amount of memory used by a process that is shared between
 *           multiple processes.
 * - Priv, private usage, the amount of memory used that is exclusively used by
 *         a process.
 * - Swap, the amount of swap memory used by a process.
 *
 * On Linux, we have two potential sources for these values, one imprecise that
 * is cheap to update and one precise that is expensive to update. Since we
 * still want to use the precise information, we need to reduce the update
 * frequency of the precise information. To improve the accuracy of the
 * imprecise information, we calculate a delta between precise and imprecise
 * information and use that as a correction factor on the imprecise information
 * if it is more recent than the precise information.
 */
struct MemoryInfo {
    inline qlonglong vmSize() const
    {
        return imprecise.other;
    }

    inline qlonglong rss() const
    {
        return value(MemoryFields::Rss);
    }

    inline qlonglong pss() const
    {
        if (precise.lastUpdate > imprecise.lastUpdate) {
            return precise.other;
        }

        if (deltas.has_value()) {
            return imprecise.priv + deltas.value().other;
        }

        return 0;
    }

    inline qlonglong shared() const
    {
        return value(MemoryFields::Shared);
    }

    inline qlonglong priv() const
    {
        return value(MemoryFields::Private);
    }

    inline qlonglong swap() const
    {
        return value(MemoryFields::Swap);
    }

    inline qlonglong value(MemoryFields::Field field) const
    {
        if (precise.lastUpdate > imprecise.lastUpdate) {
            return precise.fields[field];
        }

        if (deltas.has_value()) {
            return imprecise.fields[field] + deltas.value().fields[field];
        }

        return imprecise.fields[field];
    }

    inline void setPrecise(const MemoryFields &fields)
    {
        precise = fields;

        MemoryFields newDeltas;
        newDeltas.rss = precise.rss - imprecise.rss;
        newDeltas.shared = precise.shared - imprecise.shared;
        newDeltas.priv = precise.priv - imprecise.priv;
        newDeltas.swap = precise.swap - imprecise.swap;
        newDeltas.other = precise.other - imprecise.priv;
        newDeltas.lastUpdate = std::chrono::system_clock::now();

        deltas = newDeltas;
    }

    // Memory fields that are cheap to update but may not have great precision.
    // Other is used for vmSize.
    MemoryFields imprecise;
    // Memory fields that contain more precise information that is potentially
    // costly to retrieve, so may be updated less often. Other is used for PSS.
    MemoryFields precise;
    // Deltas between precise and imprecise fields.
    // Other is used for the delta between PSS and imprecise Private.
    // This is stored in an optional to ensure it is only set when precise
    // information has been updated.
    std::optional<MemoryFields> deltas;
};
}
