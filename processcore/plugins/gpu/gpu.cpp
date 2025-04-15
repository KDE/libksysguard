/*
    SPDX-FileCopyrightText: 2022 Lenon Kitchens <lenon.kitchens@gmail.com>
    SPDX-FileCopyrightText: 2025 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gpu.h"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>

#include <KLocalizedString>
#include <KPluginFactory>

#include <processcore/process.h>
#include <sys/stat.h>
#ifdef Q_OS_LINUX
#include <sys/sysmacros.h>
#endif
#include <unistd.h>

const fs::path proc_path{"/proc"};
const fs::path fdinfo_dir{"fdinfo"};
const fs::path fd_dir{"fd"};

const QByteArrayView engine_prefix{"drm-engine-"};
const QByteArrayView driver_prefix{"drm-driver"};
const QByteArrayView mem_resident_prefix{"drm-resident-"};
const QByteArrayView amd_resident_prefix{"drm-memory-"};
const QByteArrayView amd_drm_driver{"amdgpu"};
const QByteArrayView amd_engine{"gfx"};
const QByteArrayView intel_drm_driver{"i915"};
const QByteArrayView intel_engine{"render"};

const int32_t drm_node_type = 226;

static inline std::optional<uint64_t> to_digits(QByteArrayView s)
{
    uint64_t value;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);

    if (ec != std::errc()) {
        return {};
    }

    return value;
}

static inline float calc_gpu_usage(uint64_t curr, uint64_t prev, std::chrono::high_resolution_clock::duration diff)
{
    if (curr <= prev) {
        return 0.0F;
    }

    float perc = (static_cast<float>(curr - prev) / static_cast<float>(diff.count())) * 100.0F;

    return perc;
}

GpuPlugin::GpuPlugin(QObject *parent, const QVariantList &args)
    : ProcessDataProvider(parent, args)
{
    m_usage = new KSysGuard::ProcessAttribute(QStringLiteral("gpu_usage"), i18n("GPU Usage"), this);
    m_usage->setUnit(KSysGuard::UnitPercent);
    m_memory = new KSysGuard::ProcessAttribute(QStringLiteral("gpu_memory"), i18n("GPU Memory"), this);
    m_memory->setUnit(KSysGuard::UnitKiloByte);

    addProcessAttribute(m_usage);
    addProcessAttribute(m_memory);
}

void GpuPlugin::handleEnabledChanged(bool enabled)
{
    m_enabled = enabled;
}

bool GpuPlugin::fileRefersToDrmNode(const fs::path &path, const std::string &fname)
{
    std::string fd_path{path.string() + fd_dir.string() + "/" + fname};

    struct stat sbuf;
    if (stat(fd_path.c_str(), &sbuf) != 0) {
        return false;
    }

    if ((sbuf.st_mode & S_IFCHR) == 0) {
        return false;
    }

    return (major(sbuf.st_rdev) == drm_node_type);
}

bool GpuPlugin::processPidEntry(const fs::path &path, GpuFd &proc)
{
    QFile f{path};

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    proc.gfx = 0;
    proc.vram = 0;

    QByteArray driver;
    QHash<QByteArray, uint64_t> engineValues;

    // Had to use a do/while loop here because f.atEnd() was returning 1
    // until the first f.readLine()
    do {
        QByteArray line{f.readLine()};
        const auto separator = line.indexOf(':');
        const auto key = QByteArrayView{line.data(), separator}.trimmed();
        const auto value = QByteArrayView{line.data() + separator + 1, line.end()}.trimmed();

        if (value.contains(':')) {
            continue;
        };
        if (key == driver_prefix) {
            driver = value.toByteArray();
        } else if (key.startsWith(engine_prefix)) {
            if (const auto digits = to_digits(value)) {
                engineValues[key.mid(engine_prefix.size())] = digits.value();
            }
        } else if (key.startsWith(mem_resident_prefix) || key.startsWith(amd_resident_prefix)) {
            const auto mem = to_digits(value).value_or(0);
            // Unit can be KiB (matching the attribute), MiB or unspecified (Bytes)
            if (value.endsWith("KiB")) {
                proc.vram += mem;
            } else if (value.endsWith("Mib")) {
                proc.vram += mem * 1024;
            } else {
                proc.vram += mem / 1024;
            }
        }
    } while (!f.atEnd());

    f.close();

    if (driver == amd_drm_driver) {
        proc.gfx = engineValues[amd_engine];
    } else if (driver == intel_drm_driver) {
        proc.gfx = engineValues[intel_engine];
    }

    return (proc.gfx != 0) && (proc.vram != 0);
}

void GpuPlugin::processPidDir(const fs::path &path, KSysGuard::Process *proc, const std::unordered_map<pid_t, GpuFd> &previousValues)
{
    fs::path fdinfo_path  = path / fdinfo_dir;

    std::vector<GpuFd> gpu_fds;

    std::error_code ec;
    for (const auto &fdinfo : fs::directory_iterator(fdinfo_path, ec)) {
        if (ec != std::errc()) {
            continue;
        }

        if (fileRefersToDrmNode(path, fdinfo.path().filename().string())) {
            GpuFd gpu_fd;
            if (!processPidEntry(fdinfo.path(), gpu_fd)) {
                continue;
            }
            gpu_fds.push_back(gpu_fd);
        }
    }

    // Take the largest of all the values that we found.
    GpuFd fd_totals;
    for (auto &fd : gpu_fds) {
        if (fd.gfx > fd_totals.gfx) {
            fd_totals.gfx = fd.gfx;
        }
        if (fd.vram > fd_totals.vram) {
            fd_totals.vram = fd.vram;
        }
    }

    float usage = 0;
    if (auto it = previousValues.find(proc->pid()); it != previousValues.end()) {
        auto prev = it->second;
        usage = calc_gpu_usage(fd_totals.gfx, prev.gfx, fd_totals.ts - prev.ts);
    }

    m_process_history[proc->pid()] = fd_totals;
    m_memory->setData(proc, fd_totals.vram);
    m_usage->setData(proc, usage);
}

void GpuPlugin::update()
{
    if (!m_enabled) {
        return;
    }

    std::unordered_map<pid_t, GpuFd> previousValues;
    std::swap(previousValues, m_process_history);
    std::error_code ec;
    for (const auto &entry : fs::directory_iterator(proc_path, ec)) {
        if (ec != std::errc()) {
            continue;
        }

        QByteArray fname{entry.path().filename().c_str()};

        if (!std::all_of(fname.begin(), fname.end(), ::isdigit)) {
            continue;
        }

        const auto pid = to_digits(fname);
        if (!pid || pid == 0) {
            continue;
        }

        KSysGuard::Process *proc = getProcess(pid.value());
        if (!proc) {
            continue;
        }

        processPidDir(entry.path(), proc, previousValues);
    }
}

K_PLUGIN_FACTORY_WITH_JSON(PluginFactory, "gpu.json", registerPlugin<GpuPlugin>();)

#include "gpu.moc"
