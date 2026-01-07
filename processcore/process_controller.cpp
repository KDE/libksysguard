/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "process_controller.h"

#include <functional>

#include <QWindow>

#include <KAuth/Action>
#include <KAuth/ExecuteJob>

#include <KLocalizedString>

#include "processcore_debug.h"
#include "processes_local_p.h"

using namespace KSysGuard;

// Returns the single, unique value of the range if the range consists of several
// instances of the same unique value, or std::nullopt if there are more values.
template<typename Range, typename F, typename ResultType = std::invoke_result_t<F, typename Range::value_type>>
std::optional<ResultType> uniqueValue(const Range &range, F function)
{
    if (std::empty(range)) {
        return std::nullopt;
    }

    auto first = function(*std::begin(range));
    if (std::ranges::all_of(range, std::bind_front(std::equal_to{}, first), function)) {
        return first;
    }

    return std::nullopt;
}

struct ApplyResult {
    ProcessController::Result resultCode = ProcessController::Result::Success;
    QList<int> unchanged;
};

class ProcessController::Private
{
public:
    ApplyResult applyToPids(const QList<int> &pids, const std::function<Processes::Error(int)> &function);
    ProcessController::Result runKAuthAction(const QString &actionId, const QList<int> &pids, const QVariantMap &options);
    QList<int> listToVector(const QList<long long> &list);
    QList<int> listToVector(const QVariantList &list);

    QWindow *window = nullptr;
};

// Note: This instance is only to have access to the platform-specific code
// for sending signals, setting priority etc. Therefore, it should never be
// used to access information about processes.
Q_GLOBAL_STATIC(ProcessesLocal, s_localProcesses);

ProcessController::ProcessController(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

ProcessController::~ProcessController()
{
    // Empty destructor needed for std::unique_ptr to incomplete class.
}

QWindow *ProcessController::window() const
{
    return d->window;
}

void ProcessController::setWindow(QWindow *window)
{
    d->window = window;
}

ProcessController::Result ProcessController::sendSignal(const QList<int> &pids, int signal)
{
    qCDebug(LIBKSYSGUARD_PROCESSCORE) << "Sending signal" << signal << "to" << pids;

    auto result = d->applyToPids(pids, [signal](int pid) {
        return s_localProcesses->sendSignal(pid, signal);
    });
    if (result.unchanged.isEmpty()) {
        return result.resultCode;
    }

    return d->runKAuthAction(QStringLiteral("org.kde.ksysguard.processlisthelper.sendsignal"), result.unchanged, {{QStringLiteral("signal"), signal}});
}

ProcessController::Result ProcessController::sendSignal(const QList<long long> &pids, int signal)
{
    return sendSignal(d->listToVector(pids), signal);
}

ProcessController::Result ProcessController::sendSignal(const QVariantList &pids, int signal)
{
    return sendSignal(d->listToVector(pids), signal);
}

ProcessController::Result ProcessController::setPriority(const QList<int> &pids, int priority)
{
    auto result = d->applyToPids(pids, [priority](int pid) {
        return s_localProcesses->setNiceness(pid, priority);
    });
    if (result.unchanged.isEmpty()) {
        return result.resultCode;
    }

    return d->runKAuthAction(QStringLiteral("org.kde.ksysguard.processlisthelper.renice"), result.unchanged, {{QStringLiteral("nicevalue"), priority}});
}

ProcessController::Result ProcessController::setPriority(const QList<long long> &pids, int priority)
{
    return setPriority(d->listToVector(pids), priority);
}

ProcessController::Result ProcessController::setPriority(const QVariantList &pids, int priority)
{
    return setPriority(d->listToVector(pids), priority);
}

int ProcessController::priority(long long pid)
{
    return s_localProcesses->getNiceness(pid);
}

QJSValue ProcessController::priority(const QList<long long> &pids)
{
    return uniqueValue(pids, std::bind_front(&ProcessesLocal::getNiceness, s_localProcesses)).value_or(QJSValue::NullValue);
}

ProcessController::Result ProcessController::setCpuScheduler(const QList<int> &pids, Scheduler scheduler, int priority)
{
    if (scheduler == Scheduler::Other || scheduler == Scheduler::Batch) {
        priority = 0;
    }

    auto result = d->applyToPids(pids, [scheduler, priority](int pid) {
        return s_localProcesses->setScheduler(pid, scheduler, priority);
    });
    if (result.unchanged.isEmpty()) {
        return result.resultCode;
    }

    return d->runKAuthAction(QStringLiteral("org.kde.ksysguard.processlisthelper.changecpuscheduler"),
                             result.unchanged, //
                             {{QStringLiteral("cpuScheduler"), scheduler}, {QStringLiteral("cpuSchedulerPriority"), priority}});
}

ProcessController::Result ProcessController::setCpuScheduler(const QList<long long> &pids, Scheduler scheduler, int priority)
{
    return setCpuScheduler(d->listToVector(pids), scheduler, priority);
}

ProcessController::Result ProcessController::setCpuScheduler(const QVariantList &pids, Scheduler scheduler, int priority)
{
    return setCpuScheduler(d->listToVector(pids), scheduler, priority);
}

ProcessController::Scheduler ProcessController::cpuScheduler(long long pid)
{
    return static_cast<Scheduler>(s_localProcesses->getSchedulerClass(pid));
}

QJSValue ProcessController::cpuScheduler(const QList<long long> &pids)
{
    return uniqueValue(pids, std::bind_front(&ProcessesLocal::getSchedulerClass, s_localProcesses)).value_or(QJSValue::NullValue);
}

ProcessController::Result ProcessController::setIoScheduler(const QList<int> &pids, IoPriority priorityClass, int priority)
{
    if (!s_localProcesses->supportsIoNiceness()) {
        return Result::Unsupported;
    }

    if (priorityClass == IoPriority::Idle) {
        priority = 0;
    }

    auto result = d->applyToPids(pids, [priorityClass, priority](int pid) {
        return s_localProcesses->setIoNiceness(pid, priorityClass, priority);
    });
    if (result.unchanged.isEmpty()) {
        return result.resultCode;
    }

    return d->runKAuthAction(QStringLiteral("org.kde.ksysguard.processlisthelper.changeioscheduler"),
                             result.unchanged, //
                             {{QStringLiteral("ioScheduler"), priorityClass}, {QStringLiteral("ioSchedulerPriority"), priority}});
}

ProcessController::Result ProcessController::setIoScheduler(const QList<long long> &pids, IoPriority priorityClass, int priority)
{
    return setIoScheduler(d->listToVector(pids), priorityClass, priority);
}

ProcessController::Result ProcessController::setIoScheduler(const QVariantList &pids, IoPriority priorityClass, int priority)
{
    return setIoScheduler(d->listToVector(pids), priorityClass, priority);
}

int ProcessController::ioPriority(long long pid)
{
    return s_localProcesses->getIoNiceness(pid);
}

QJSValue ProcessController::ioPriority(const QList<long long> &pids)
{
    return uniqueValue(pids, std::bind_front(&ProcessesLocal::getIoNiceness, s_localProcesses)).value_or(QJSValue::NullValue);
}

ProcessController::IoPriority ProcessController::ioScheduler(long long pid)
{
    return static_cast<IoPriority>(s_localProcesses->getIoPriorityClass(pid));
}

QJSValue ProcessController::ioScheduler(const QList<long long> &pids)
{
    return uniqueValue(pids, std::bind_front(&ProcessesLocal::getIoPriorityClass, s_localProcesses)).value_or(QJSValue::NullValue);
}

QString ProcessController::resultToString(Result result)
{
    switch (result) {
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

ApplyResult ProcessController::Private::applyToPids(const QList<int> &pids, const std::function<Processes::Error(int)> &function)
{
    ApplyResult result;

    for (auto pid : pids) {
        auto error = function(pid);
        switch (error) {
        case Processes::InsufficientPermissions:
        case Processes::Unknown:
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
        case Processes::NoError:
            break;
        }
    }
    return result;
}

ProcessController::Result ProcessController::Private::runKAuthAction(const QString &actionId, const QList<int> &pids, const QVariantMap &options)
{
    KAuth::Action action(actionId);
    if (!action.isValid()) {
        qCWarning(LIBKSYSGUARD_PROCESSCORE) << "Executing KAuth action" << actionId << "failed because it is an invalid action";
        return Result::InsufficientPermissions;
    }
    action.setParentWindow(window);
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
    if (job->exec()) {
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

QList<int> ProcessController::Private::listToVector(const QList<long long> &list)
{
    QList<int> vector;
    std::transform(list.cbegin(), list.cend(), std::back_inserter(vector), [](long long entry) {
        return entry;
    });
    return vector;
}

QList<int> ProcessController::Private::listToVector(const QVariantList &list)
{
    QList<int> vector;
    std::transform(list.cbegin(), list.cend(), std::back_inserter(vector), [](const QVariant &entry) {
        return entry.toInt();
    });
    return vector;
}
