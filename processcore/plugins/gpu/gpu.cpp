/*
    SPDX-FileCopyrightText: 2022 Lenon Kitchens <lenon.kitchens@gmail.com>
    SPDX-FileCopyrightText: 2025 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gpu.h"

#include <algorithm>
#include <charconv>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <QDebug>
#include <QFile>
#include <QHash>
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
#include <xf86drm.h>

using namespace Qt::StringLiterals;

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

const int nvidiaVendorId = 0x10de;

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

static std::optional<uint> drmMinor(const fs::path &path)
{
    struct stat sbuf;
    if (stat(path.string().c_str(), &sbuf) != 0) {
        return {};
    }

    if ((sbuf.st_mode & S_IFCHR) == 0) {
        return {};
    }

    if ((major(sbuf.st_rdev) != drm_node_type)) {
        return {};
    };
    return minor(sbuf.st_rdev);
}

GpuPlugin::GpuPlugin(QObject *parent, const QVariantList &args)
    : ProcessDataProvider(parent, args)
    , m_sniExecutablePath(QStandardPaths::findExecutable(QStringLiteral("nvidia-smi")))
{
    m_usage = new KSysGuard::ProcessAttribute(QStringLiteral("gpu_usage"), i18n("GPU Usage"), this);
    m_usage->setUnit(KSysGuard::UnitPercent);
    m_memory = new KSysGuard::ProcessAttribute(QStringLiteral("gpu_memory"), i18n("GPU Memory"), this);
    m_memory->setUnit(KSysGuard::UnitKiloByte);
    m_gpuName = new KSysGuard::ProcessAttribute(QStringLiteral("gpu_module"), i18n("GPU"), this);
    m_gpuName->setDescription(i18n("Displays which GPU the process is using"));

    addProcessAttribute(m_usage);
    addProcessAttribute(m_memory);
    addProcessAttribute(m_gpuName);

    std::vector<drmDevicePtr> devices;
    const int count = drmGetDevices2(0, nullptr, 0);
    devices.resize(count);
    std::vector<GpuInfo> nvidiaGpus;
    if (drmGetDevices2(0, devices.data(), count) > 0) {
        for (const auto &device : devices) {
            if (auto minor = drmMinor(device->nodes[DRM_NODE_PRIMARY])) {
                m_minorToGpuNum[*minor] = *minor;
                if (auto renderMinor = drmMinor(device->nodes[DRM_NODE_RENDER])) {
                    m_minorToGpuNum[*renderMinor] = *minor;
                }
                if (device->bustype == DRM_BUS_PCI && device->deviceinfo.pci->vendor_id == nvidiaVendorId) {
                    auto pciAddress = std::format("{:08x}:{:02x}:{:02x}.{:x}",
                                                  device->businfo.pci->domain,
                                                  device->businfo.pci->bus,
                                                  device->businfo.pci->dev,
                                                  device->businfo.pci->func);
                    nvidiaGpus.emplace_back(pciAddress, *minor);
                }
            }
        }
    }

    if (nvidiaGpus.size() > 0 && !m_sniExecutablePath.isEmpty()) {
        setupNvidia(nvidiaGpus);
    }
    drmFreeDevices(devices.data(), devices.size());
}

GpuPlugin::~GpuPlugin() noexcept
{
    if (m_nvidiaSmiProcess) {
        m_nvidiaSmiProcess->terminate();
        m_nvidiaSmiProcess->waitForFinished();
    }
}

void GpuPlugin::setupNvidia(const std::vector<GpuInfo> &gpuInfo)
{
    auto nvidiaQuery = QProcess();
    nvidiaQuery.start(m_sniExecutablePath, {"--query-gpu=pci.bus_id,index"_L1, "--format=csv,noheader"_L1});
    while (nvidiaQuery.waitForReadyRead()) {
        if (!nvidiaQuery.canReadLine()) {
            continue;
        }
        const auto line = nvidiaQuery.readLine().split(u',');
        if (auto gpuNum = std::ranges::find(gpuInfo, QByteArrayView(line[0]), &GpuInfo::pciAdress); gpuNum != gpuInfo.end()) {
            m_nvidiaIndexToGpuNum.emplace(line[1].toUInt(), gpuNum->deviceMinor);
        }
        m_nvidiaSmiProcess = new QProcess;
        m_nvidiaSmiProcess->setProgram(m_sniExecutablePath);
        m_nvidiaSmiProcess->setArguments({QStringLiteral("pmon"), QStringLiteral("-s"), QStringLiteral("mu")});
        connect(m_nvidiaSmiProcess, &QProcess::readyReadStandardOutput, this, &GpuPlugin::readNvidiaData);
    }
}

void GpuPlugin::handleEnabledChanged(bool enabled)
{
    m_enabled = enabled;
    if (!m_nvidiaSmiProcess) {
        return;
    }
    if (enabled) {
        if (m_nvidiaIndexToGpuNum.size() > 0) {
            m_nvidiaSmiProcess->start();
        }
    } else {
        m_nvidiaSmiProcess->terminate();
    }
}

struct pmonIndices {
    int pid = -1;
    int index = -1;
    int sm = -1;
    int fb = -1;
};

void GpuPlugin::readNvidiaData()
{
    static pmonIndices indices;

    while (m_nvidiaSmiProcess->canReadLine()) {
        const QString line = QString::fromLatin1(m_nvidiaSmiProcess->readLine());
        auto parts = QStringView(line).split(u' ', Qt::SkipEmptyParts);
        // discover index of fields in the header format is something like
        // # gpu         pid   type     fb   ccpm     sm    mem    enc    dec    jpg    ofa    command
        // # Idx           #    C/G     MB     MB      %      %      %      %      %      %    name
        //     0       1424     G     15      0      -      -      -      -      -      -    Xorg
        if (line.startsWith(u'#')) { // comment line
            if (indices.pid == -1) {
                // Remove First part because of leading '# ';
                parts.removeFirst();
                indices.index = parts.indexOf("gpu"_L1);
                indices.pid = parts.indexOf("pid"_L1);
                indices.sm = parts.indexOf("sm"_L1);
                indices.fb = parts.indexOf("fb"_L1);
            }
            continue;
        }

        if (indices.pid == -1 || indices.index == -1) {
            m_nvidiaSmiProcess->terminate();
            continue;
        }

        pid_t pid = parts[indices.pid].toUInt();
        unsigned int index = parts[indices.index].toUInt();
        unsigned int sm = indices.sm >= 0 ? parts[indices.sm].toUInt() : 0;
        unsigned int mem = indices.fb >= 0 ? parts[indices.fb].toUInt() * 1024 : 0;
        if (auto device = m_nvidiaIndexToGpuNum.find(index); device != m_nvidiaIndexToGpuNum.end()) {
            m_currentNvidiaValues[{pid, device->second}] = {sm, mem};
        }
    }
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

void GpuPlugin::processPidDir(const fs::path &path, KSysGuard::Process *proc, const std::unordered_map<HistoryKey, GpuFd> &previousValues)
{
    fs::path fdinfo_path  = path / fdinfo_dir;

    std::unordered_map<HistoryKey, GpuFd> gpu_fds;

    std::error_code ec;
    for (const auto &fdinfo : fs::directory_iterator(fdinfo_path, ec)) {
        if (ec != std::errc()) {
            continue;
        }

        if (auto device = drmMinor(path / fd_dir / fdinfo.path().filename())) {
            if (gpu_fds.contains(HistoryKey(proc->pid(), *device))) {
                continue;
            }
            GpuFd gpu_fd;
            gpu_fd.ts = std::chrono::high_resolution_clock::now();
            if (!processPidEntry(fdinfo.path(), gpu_fd)) {
                continue;
            }
            gpu_fds.emplace(HistoryKey(proc->pid(), *device), gpu_fd);
        }
    }

    float usage = 0;
    uint32_t vram = 0;
    int device = -1;

    for (const auto &value : gpu_fds) {
        if (auto it = previousValues.find(value.first); it != previousValues.end()) {
            auto prev = it->second;
            const auto deviceUsage = calc_gpu_usage(value.second.gfx, prev.gfx, value.second.ts - prev.ts);
            if (deviceUsage > usage) {
                usage = deviceUsage;
                device = value.first.deviceMinor;
                vram = value.second.vram;
            } else if (usage == 0 && value.second.vram > vram) {
                device = value.first.deviceMinor;
                vram = value.second.vram;
            }
        }
    }

    for (const auto &nvidiaGpu : m_nvidiaIndexToGpuNum) {
        if (auto values = m_currentNvidiaValues.find(HistoryKey(proc->pid(), nvidiaGpu.second)); values != m_currentNvidiaValues.end()) {
            if (values->second.usage > usage) {
                usage = values->second.usage;
                vram = values->second.vram;
                device = nvidiaGpu.second;
            } else if (usage == 0 && values->second.vram > vram) {
                vram = values->second.vram;
                device = nvidiaGpu.second;
            }
        }
    }

    m_process_history.merge(gpu_fds);
    m_memory->setData(proc, vram);
    m_usage->setData(proc, usage);
    if (device != -1) {
        auto gpu = m_minorToGpuNum.find(device);
        if (gpu != m_minorToGpuNum.end()) {
            // Match ksystemstats gpu plugin
            m_gpuName->setData(proc, i18nc("%1 is a number", "GPU %1", gpu->second + 1));
        }
    }
}

void GpuPlugin::update()
{
    if (!m_enabled) {
        return;
    }
    std::unordered_map<HistoryKey, GpuFd> previousValues;
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
    m_currentNvidiaValues.clear();
}

K_PLUGIN_FACTORY_WITH_JSON(PluginFactory, "gpu.json", registerPlugin<GpuPlugin>();)

#include "gpu.moc"
