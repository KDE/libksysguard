/*
    SPDX-FileCopyrightText: 2022 Lenon Kitchens <lenon.kitchens@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <chrono>
#include <filesystem>
#include <unordered_map>
#include <vector>

#include <processcore/process_attribute.h>
#include <processcore/process_data_provider.h>

namespace fs = std::filesystem;

struct GpuFd {
    pid_t pid{0};
    uint64_t gfx{0};
    uint32_t vram{0};
    std::chrono::high_resolution_clock::time_point ts{std::chrono::high_resolution_clock::now()};
};

class GpuPlugin : public KSysGuard::ProcessDataProvider
{
    Q_OBJECT
public:
    GpuPlugin(QObject *parent, const QVariantList &args);
    void handleEnabledChanged(bool enabled) override;
    void update() override;

private:
    KSysGuard::ProcessAttribute *m_usage = nullptr;
    KSysGuard::ProcessAttribute *m_memory = nullptr;

    bool m_enabled = false;
    std::unordered_map<pid_t, GpuFd> m_process_history;

    void processPidDir(const pid_t p, const fs::path &path, KSysGuard::Process *proc, const std::unordered_map<pid_t, GpuFd> &previousValues);
    bool processPidEntry(const fs::path &path, GpuFd &proc);
    bool fileRefersToDrmNode(const fs::path &path, const std::string &fname);
};
