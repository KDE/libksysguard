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

const QByteArrayView driver_prefix{"drm-driver"};
const QByteArrayView gfx_prefix{"drm-engine-gfx"};
const QByteArrayView mem_prefix{"drm-memory-vram"};
const QByteArrayView amd_drm_driver{"amdgpu"};

const int32_t drm_node_type = 226;

template<class T>
static inline bool to_digits(QByteArrayView s, T &v)
{
    auto [ptr, ec]{std::from_chars(s.data(), s.data() + s.size(), v)};

    if (ec != std::errc()) {
        return false;
    }

    return true;
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
    m_usage = new KSysGuard::ProcessAttribute(QStringLiteral("amdgpu_usage"), i18n("GPU Usage"), this);
    m_usage->setUnit(KSysGuard::UnitPercent);
    m_memory = new KSysGuard::ProcessAttribute(QStringLiteral("amdgpu_memory"), i18n("GPU Memory"), this);
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

    // Had to use a do/while loop here because f.atEnd() was returning 1
    // until the first f.readLine()
    do {
        QByteArray line{f.readLine()};
        const auto separator = line.indexOf(':');
        const auto key = QByteArrayView{line.data(), separator}.trimmed();
        const auto value = QByteArrayView{line.data(), separator}.trimmed();

        if (value.contains(':')) {
            continue;
        };

        if (key == driver_prefix) {
            if (value != amd_drm_driver) {
                break;
            };
        } else if (key == gfx_prefix) {
            if (!to_digits(value, proc.gfx)) {
                continue;
            };
        } else if (key == mem_prefix) {
            if (!to_digits(value, proc.vram)) {
                continue;
            };
        }
    } while (!f.atEnd());

    f.close();

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

/*
    For each pid entry in proc, scan the fdinfo directory for entries containing both
    drm-driver: amdgpu and drm-engine-gfx: <999999999> ns.  When found, determine the
    elapsed ns since the last reading and compute a GPU usage percentage from there.
    Be care of multiple entries in fdinfo matching the criteria.
*/
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
