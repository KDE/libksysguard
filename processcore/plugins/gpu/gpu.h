/*
    SPDX-FileCopyrightText: 2022 Lenon Kitchens <lenon.kitchens@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QProcess>

#include <chrono>
#include <filesystem>
#include <unordered_map>
#include <vector>

#include <processcore/process_attribute.h>
#include <processcore/process_data_provider.h>

namespace fs = std::filesystem;

struct GpuFd {
    uint64_t gfx{0};
    uint32_t vram{0};
    std::chrono::high_resolution_clock::time_point ts;
};

struct HistoryKey {
    pid_t pid;
    uint64_t deviceMinor;
    friend bool operator==(const HistoryKey &, const HistoryKey &) = default;
};

struct NvidiaValues {
    unsigned int usage = 0;
    unsigned int vram = 0;
};

struct GpuInfo {
    std::string pciAdress;
    unsigned int deviceMinor;
};

template<>
struct std::hash<HistoryKey> {
    std::size_t operator()(const HistoryKey &key, size_t seed = 0) const noexcept
    {
        return qHashMulti(seed, key.pid, key.deviceMinor);
    }
};

class GpuPlugin : public KSysGuard::ProcessDataProvider
{
    Q_OBJECT
public:
    GpuPlugin(QObject *parent, const QVariantList &args);
    ~GpuPlugin();
    void handleEnabledChanged(bool enabled) override;
    void update() override;
    void readNvidiaData();

private:
    KSysGuard::ProcessAttribute *m_usage = nullptr;
    KSysGuard::ProcessAttribute *m_memory = nullptr;
    KSysGuard::ProcessAttribute *m_gpuName = nullptr;

    bool m_enabled = false;
    QString m_sniExecutablePath;
    QProcess *m_nvidiaSmiProcess = nullptr;

    std::unordered_map<HistoryKey, GpuFd> m_process_history;
    std::unordered_map<HistoryKey, NvidiaValues> m_currentNvidiaValues;
    std::unordered_map<unsigned int, unsigned int> m_minorToGpuNum;
    std::unordered_map<unsigned int, unsigned int> m_nvidiaIndexToGpuNum;

    void processPidDir(const fs::path &path, KSysGuard::Process *proc, const std::unordered_map<HistoryKey, GpuFd> &previousValues);
    bool processPidEntry(const fs::path &path, GpuFd &proc);
    void setupNvidia(const std::vector<GpuInfo> &gpuInfo);
};
