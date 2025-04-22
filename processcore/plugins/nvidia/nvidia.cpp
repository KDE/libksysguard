/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "nvidia.h"

#include <QDebug>
#include <QProcess>
#include <QStandardPaths>

#include <KLocalizedString>
#include <KPluginFactory>

#include <processcore/process.h>

using namespace KSysGuard;
using namespace Qt::StringLiterals;

NvidiaPlugin::NvidiaPlugin(QObject *parent, const QVariantList &args)
    : ProcessDataProvider(parent, args)
    , m_sniExecutablePath(QStandardPaths::findExecutable(QStringLiteral("nvidia-smi")))
{
    if (m_sniExecutablePath.isEmpty()) {
        return;
    }

    m_usage = new ProcessAttribute(QStringLiteral("nvidia_usage"), i18n("GPU Usage"), this);
    m_usage->setUnit(KSysGuard::UnitPercent);
    m_memory = new ProcessAttribute(QStringLiteral("nvidia_memory"), i18n("GPU Memory"), this);
    m_memory->setUnit(KSysGuard::UnitMegaByte);

    connect(processes(), &Processes::beginAddProcess, this, [this](Process *process) {
        m_usage->setData(process, 0);
        m_memory->setData(process, 0);
    });

    addProcessAttribute(m_usage);
    addProcessAttribute(m_memory);
}

void NvidiaPlugin::handleEnabledChanged(bool enabled)
{
    if (enabled) {
        if (!m_process) {
            setup();
        }
        m_process->start();
    } else {
        if (m_process) {
            m_process->terminate();
        }
    }
}

struct pmonIndices {
    int pid = -1;
    int sm = -1;
    int fb = -1;
};

void NvidiaPlugin::setup()
{
    m_process = new QProcess(this);
    m_process->setProgram(m_sniExecutablePath);
    m_process->setArguments({QStringLiteral("pmon"), QStringLiteral("-s"), QStringLiteral("um")});

    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        static pmonIndices indices;

        while (m_process->canReadLine()) {
            const QString line = QString::fromLatin1(m_process->readLine());
            auto parts = QStringView(line).split(u' ', Qt::SkipEmptyParts);
            // discover index of fields in the header format is something like
            // # gpu         pid   type     fb   ccpm     sm    mem    enc    dec    jpg    ofa    command
            // # Idx           #    C/G     MB     MB      %      %      %      %      %      %    name
            //     0       1424     G     15      0      -      -      -      -      -      -    Xorg
            if (line.startsWith(u'#')) { // comment line
                if (indices.pid == -1) {
                    // Remove First part because of leading '# ';
                    parts.removeFirst();
                    indices.pid = parts.indexOf("pid"_L1);
                    indices.sm = parts.indexOf("sm"_L1);
                    indices.fb = parts.indexOf("fb"_L1);
                }
                continue;
            }

            if (indices.pid == -1) {
                m_process->terminate();
                continue;
            }

            long pid = parts[indices.pid].toUInt();
            int sm = indices.sm >= 0 ? parts[indices.sm].toUInt() : 0;
            int mem = indices.mem >= 0 ? parts[indices.fb].toUInt() : 0;

            KSysGuard::Process *process = getProcess(pid);
            if (!process) {
                continue; // can in race condition etc
            }

            m_usage->setData(process, sm);
            m_memory->setData(process, mem);
        }
    });
}

K_PLUGIN_FACTORY_WITH_JSON(PluginFactory, "nvidia.json", registerPlugin<NvidiaPlugin>();)

#include "nvidia.moc"
