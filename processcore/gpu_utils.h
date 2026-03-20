/*
    SPDX-FileCopyrightText: 2026 Bernhard Friedreich <friesoft@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSYSGUARD_GPU_UTILS_H
#define KSYSGUARD_GPU_UTILS_H

#include <QString>
#include "processcore_export.h"

struct udev_device;

namespace KSysGuard {

/**
 * Returns the GPU name for the given udev device.
 * It handles both PCI and DRM devices. For DRM devices, it will automatically
 * look up the parent PCI device to get a better name.
 * Falls back to "GPU %1" using @p fallbackIndex + 1 if no name is found.
 */
PROCESSCORE_EXPORT QString gpuName(struct udev_device *device, int fallbackIndex);

/**
 * Returns a fallback GPU name using an index.
 * e.g. "GPU 1"
 */
PROCESSCORE_EXPORT QString gpuNameFallback(int index);

}

#endif
