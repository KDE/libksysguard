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
        Pss,
        Shared,
        Private,
        Swap,
    };

    inline qlonglong value(Field field) const
    {
        switch (field) {
        case Rss:
            return rss;
        case Pss:
            return pss;
        case Shared:
            return shared;
        case Private:
            return priv;
        case Swap:
            return swap;
        }

        return -1;
    }

    std::chrono::steady_clock::time_point lastUpdate;

    qlonglong rss = -1;
    qlonglong pss = -1;
    qlonglong shared = -1;
    qlonglong priv = -1;
    qlonglong swap = -1;
};

/*
 * A struct to keep track of memory information.
 *
 * There are a number of values we want to track for memory usage:
 *
 * - RSS: resident set size; the amount of memory used by a process including
 *        any shared memory.
 * - PSS: proportional set size; the amount of memory used by a process, including
 *        the proportion of shared memory that specific process contributes.
 * - Shared: the amount of memory used by a process that is shared between
 *           multiple processes.
 * - Priv: private usage the amount of memory used that is exclusively used by
 *         a process.
 * - Swap: the amount of swap memory used by a process.
 *
 * On Linux, we have two potential sources for these values: one imprecise that
 * is cheap to update, and one precise that is expensive to update. While we
 * still want to use the precise information, we need to reduce its update
 * frequency so that we use less processing power. To improve the accuracy of the
 * imprecise information, we calculate a delta between precise and imprecise
 * information, and use that as a correction factor on the imprecise information
 * if it is more recent than the precise information.
 */
struct MemoryInfo {
    inline qlonglong rss() const
    {
        return value(MemoryFields::Rss);
    }

    inline qlonglong pss() const
    {
        if (precise.lastUpdate > imprecise.lastUpdate) {
            return precise.pss;
        }

        if (deltas.has_value()) {
            return std::max(imprecise.priv + deltas.value().pss, 0ll);
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
            return precise.value(field);
        }

        if (deltas.has_value()) {
            return std::max(imprecise.value(field) + deltas.value().value(field), 0ll);
        }

        return imprecise.value(field);
    }

    inline void setPrecise(const MemoryFields &fields)
    {
        precise = fields;

        MemoryFields newDeltas;
        newDeltas.rss = precise.rss - imprecise.rss;
        // We don't have an "imprecise pss", instead we use the delta between
        // PSS and imprecise private usage to calculate an updated PSS.
        newDeltas.pss = precise.pss - imprecise.priv;
        newDeltas.shared = precise.shared - imprecise.shared;
        newDeltas.priv = precise.priv - imprecise.priv;
        newDeltas.swap = precise.swap - imprecise.swap;
        newDeltas.lastUpdate = std::chrono::steady_clock::now();

        deltas = newDeltas;
    }

    qlonglong vmSize = -1;

    // Memory fields that are cheap to update but may not have great precision.
    MemoryFields imprecise;
    // Memory fields that contain more precise information that is potentially
    // costly to retrieve, so may be updated less often.
    MemoryFields precise;
    // Deltas between precise and imprecise fields.
    // This is stored in an optional to ensure it is only set when precise
    // information has been updated.
    std::optional<MemoryFields> deltas;
};
}
