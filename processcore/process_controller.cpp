/*
 * This file is part of KSysGuard.
 * Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "process_controller.h"

#include <functional>

#include <KLocalizedString>
#include <KAuth/KAuthAction>
#include <KAuth/KAuthExecuteJob>

#include "processes_local_p.h"
#include "processcore_debug.h"

using namespace KSysGuard;

struct ApplyResult
{
    ProcessController::Result resultCode = ProcessController::Result::Success;
    QVector<int> unchanged;
};

class ProcessController::Private
{
public:
    ApplyResult applyToPids(const QVector<int> &pids, const std::function<bool(int)> &function);
    ProcessController::Result runKAuthAction(const QString &actionId, const QVector<int> &pids, const QVariantMap &options);
    QVector<int> listToVector(const QList<long long> &list);
    QVector<int> listToVector(const QVariantList &list);

    QWidget *widget;

    // Note: This instance is only to have access to the platform-specific code
    // for sending signals, setting priority etc. Therefore, it should never be
    // used to access information about processes.
    static std::unique_ptr<ProcessesLocal> localProcesses;
};

std::unique_ptr<ProcessesLocal> ProcessController::Private::localProcesses;

ProcessController::ProcessController(QObject* parent)
    : QObject(parent), d(new Private)
{
    if (!d->localProcesses) {
        d->localProcesses = std::make_unique<ProcessesLocal>();
    }
}

KSysGuard::ProcessController::~ProcessController()
{
    // Empty destructor needed for std::unique_ptr to incomplete class.
}

QWidget * KSysGuard::ProcessController::widget() const
{
    return d->widget;
}

void KSysGuard::ProcessController::setWidget(QWidget* widget)
{
    d->widget = widget;
}

ProcessController::Result ProcessController::sendSignal(const QVector<int>& pids, int signal)
{
    qCDebug(LIBKSYSGUARD_PROCESSCORE) << "Sending signal" << signal << "to" << pids;

    auto result = d->applyToPids(pids, [this, signal](int pid) { return d->localProcesses->sendSignal(pid, signal); });
    if (result.unchanged.isEmpty()) {
        return result.resultCode;
    }

    return d->runKAuthAction(
        QStringLiteral("org.kde.ksysguard.processlisthelper.sendsignal"),
        result.unchanged,
        { {QStringLiteral("signal"), signal} }
    );
}

KSysGuard::ProcessController::Result KSysGuard::ProcessController::sendSignal(const QList<long long>& pids, int signal)
{
    return sendSignal(d->listToVector(pids), signal);
}

KSysGuard::ProcessController::Result KSysGuard::ProcessController::sendSignal(const QVariantList &pids, int signal)
{
    return sendSignal(d->listToVector(pids), signal);
}

ProcessController::Result ProcessController::setPriority(const QVector<int>& pids, int priority)
{
    auto result = d->applyToPids(pids, [this, priority](int pid) { return d->localProcesses->setNiceness(pid, priority); });
    if (result.unchanged.isEmpty()) {
        return result.resultCode;
    }

    return d->runKAuthAction(
        QStringLiteral("org.kde.ksysguard.processlisthelper.renice"),
        result.unchanged,
        { { QStringLiteral("nicevalue"), priority } }
    );
}

KSysGuard::ProcessController::Result KSysGuard::ProcessController::setPriority(const QList<long long>& pids, int priority)
{
    return setPriority(d->listToVector(pids), priority);
}

KSysGuard::ProcessController::Result KSysGuard::ProcessController::setPriority(const QVariantList &pids, int priority)
{
    return setPriority(d->listToVector(pids), priority);
}

ProcessController::Result ProcessController::setCPUScheduler(const QVector<int>& pids, Process::Scheduler scheduler, int priority)
{
    if (scheduler == KSysGuard::Process::Other || scheduler == KSysGuard::Process::Batch) {
        priority = 0;
    }

    auto result = d->applyToPids(pids, [this, scheduler, priority](int pid) {
        return d->localProcesses->setScheduler(pid, scheduler, priority);
    });
    if (result.unchanged.isEmpty()) {
        return result.resultCode;
    }

    return d->runKAuthAction(
        QStringLiteral("org.kde.ksysguard.processlisthelper.changecpuscheduler"),
        result.unchanged,
        {{QStringLiteral("cpuScheduler"), scheduler}, {QStringLiteral("cpuSchedulerPriority"), priority}}
    );
}

KSysGuard::ProcessController::Result KSysGuard::ProcessController::setCPUScheduler(const QList<long long>& pids, Process::Scheduler scheduler, int priority)
{
    return setCPUScheduler(d->listToVector(pids), scheduler, priority);
}

KSysGuard::ProcessController::Result KSysGuard::ProcessController::setCPUScheduler(const QVariantList &pids, Process::Scheduler scheduler, int priority)
{
    return setCPUScheduler(d->listToVector(pids), scheduler, priority);
}

ProcessController::Result ProcessController::setIOScheduler(const QVector<int>& pids, Process::IoPriorityClass priorityClass, int priority)
{
    if (!d->localProcesses->supportsIoNiceness()) {
        return Result::Unsupported;
    }

    if (priorityClass == KSysGuard::Process::None) {
        priorityClass = KSysGuard::Process::BestEffort;
    }

    if (priorityClass == KSysGuard::Process::Idle) {
        priority = 0;
    }

    auto result = d->applyToPids(pids, [this, priorityClass, priority](int pid) {
        return d->localProcesses->setIoNiceness(pid, priorityClass, priority);
    });
    if (result.unchanged.isEmpty()) {
        return result.resultCode;
    }

    return d->runKAuthAction(
        QStringLiteral("org.kde.ksysguard.processlisthelper.changeioscheduler"),
        result.unchanged,
        {{QStringLiteral("ioScheduler"), priorityClass}, {QStringLiteral("ioSchedulerPriority"), priority}}
    );
}

KSysGuard::ProcessController::Result KSysGuard::ProcessController::setIOScheduler(const QList<long long>& pids, Process::IoPriorityClass priorityClass, int priority)
{
    return setIOScheduler(d->listToVector(pids), priorityClass, priority);
}

KSysGuard::ProcessController::Result KSysGuard::ProcessController::setIOScheduler(const QVariantList &pids, Process::IoPriorityClass priorityClass, int priority)
{
    return setIOScheduler(d->listToVector(pids), priorityClass, priority);
}

QString ProcessController::resultToString(Result result)
{
    switch(result) {
    case Result::Success:
        return i18n("Success");
    case Result::InsufficientPermissions:
        return i18n("Insufficient permissions.");
    case Result::NoSuchProcess:
        return i18n("No matching process was found.");
    case Result::Unsupported:
        return i18n("Not supported on the current system.");
    case Result::UserCancelled:
        return i18n("The user cancelled.");
    case Result::Error:
        return i18n("An unspecified error occurred.");
    default:
        return i18n("An unknown error occurred.");
    }
}

ApplyResult KSysGuard::ProcessController::Private::applyToPids(const QVector<int>& pids, const std::function<bool(int)>& function)
{
    ApplyResult result;

    localProcesses->errorCode = KSysGuard::Processes::Unknown;

    for (auto pid : pids) {
        auto success = function(pid);
        if (!success) {
            switch (localProcesses->errorCode) {
            case KSysGuard::Processes::InsufficientPermissions:
            case KSysGuard::Processes::Unknown:
                result.unchanged << pid;
                result.resultCode = Result::InsufficientPermissions;
                break;
            case Processes::InvalidPid:
            case Processes::ProcessDoesNotExistOrZombie:
            case Processes::InvalidParameter:
                result.resultCode = Result::NoSuchProcess;
                break;
            case Processes::NotSupported:
                result.resultCode = Result::Unsupported;
                break;
            default:
                result.resultCode = Result::Unknown;
                break;
            }
        }
    }
    return result;
}


ProcessController::Result ProcessController::Private::runKAuthAction(const QString& actionId, const QVector<int> &pids, const QVariantMap& options)
{
    KAuth::Action action(actionId);
    if (!action.isValid()) {
        qCWarning(LIBKSYSGUARD_PROCESSCORE) << "Executing KAuth action" << actionId << "failed because it is an invalid action";
        return Result::InsufficientPermissions;
    }
    action.setParentWidget(widget);
    action.setHelperId(QStringLiteral("org.kde.ksysguard.processlisthelper"));

    const int processCount = pids.count();
    for (int i = 0; i < processCount; ++i) {
        action.addArgument(QStringLiteral("pid%1").arg(i), pids.at(i));
    }
    action.addArgument(QStringLiteral("pidcount"), processCount);

    for (auto itr = options.cbegin(); itr != options.cend(); ++itr) {
        action.addArgument(itr.key(), itr.value());
    }

    KAuth::ExecuteJob *job = action.execute();
    if(job->exec()) {
        return Result::Success;
    } else {
        if (job->error() == KAuth::ActionReply::UserCancelledError) {
            return Result::UserCancelled;
        }

        if (job->error() == KAuth::ActionReply::AuthorizationDeniedError) {
            return Result::InsufficientPermissions;
        }

        qCWarning(LIBKSYSGUARD_PROCESSCORE) << "Executing KAuth action" << actionId << "failed with error code" << job->error();
        qCWarning(LIBKSYSGUARD_PROCESSCORE) << job->errorString();
        return Result::Error;
    }
}

QVector<int> KSysGuard::ProcessController::Private::listToVector(const QList<long long>& list)
{
    QVector<int> vector;
    std::transform(list.cbegin(), list.cend(), std::back_inserter(vector), [](long long entry) { return entry; });
    return vector;
}

QVector<int> KSysGuard::ProcessController::Private::listToVector(const QVariantList &list)
{
    QVector<int> vector;
    std::transform(list.cbegin(), list.cend(), std::back_inserter(vector), [](const QVariant &entry) { return entry.toInt(); });
    return vector;
}
