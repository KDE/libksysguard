/*
    SPDX-FileCopyrightText: 2026 Bernhard Friedreich <friesoft@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gpu_utils.h"

#ifdef HAVE_UDEV
#include <libudev.h>
#endif

#include <KLocalizedString>

namespace KSysGuard {

QString gpuNameFallback(int index)
{
    return i18nc("@title %1 is GPU number", "GPU %1", index + 1);
}

#ifdef HAVE_UDEV
static QString getRawName(struct udev_device *device)
{
    const char *model = udev_device_get_property_value(device, "ID_MODEL_FROM_DATABASE");
    if (!model) {
        model = udev_device_get_property_value(device, "ID_MODEL");
    }

    if (model) {
        return QString::fromLocal8Bit(model);
    }

    const char *vendorFromDb = udev_device_get_property_value(device, "ID_VENDOR_FROM_DATABASE");
    if (vendorFromDb) {
        return i18nc("@title %1 is vendor name", "%1 GPU", QString::fromLocal8Bit(vendorFromDb));
    }

    return {};
}

QString gpuName(struct udev_device *device, int fallbackIndex)
{
    if (!device) {
        return gpuNameFallback(fallbackIndex);
    }

    const char *subsystem = udev_device_get_subsystem(device);
    struct udev_device *pciDevice = nullptr;
    if (subsystem && std::string_view(subsystem) == "pci") {
        pciDevice = device;
    } else {
        pciDevice = udev_device_get_parent_with_subsystem_devtype(device, "pci", nullptr);
    }

    QString name;
    if (pciDevice) {
        name = getRawName(pciDevice);
    }

    if (name.isEmpty()) {
        name = getRawName(device);
    }

    if (name.isEmpty()) {
        struct udev_device *parent = udev_device_get_parent(device);
        if (parent) {
            name = getRawName(parent);
        }
    }

    if (name.isEmpty()) {
        return gpuNameFallback(fallbackIndex);
    }
    return name;
}
#else
QString gpuName(struct udev_device *, int fallbackIndex) { return gpuNameFallback(fallbackIndex); }
#endif

}
