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

KSysGuard::ProcessController::~ProcessController()
{
    // Empty destructor needed for std::unique_ptr to incomplete class.
}

QWindow *KSysGuard::ProcessController::window() const
{
    return d->window;
}

void KSysGuard::ProcessController::setWindow(QWindow *window)
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

KSysGuard::ProcessController::Result KSysGuard::ProcessController::sendSignal(const QList<long long> &pids, int signal)
{
    return sendSignal(d->listToVector(pids), signal);
}

KSysGuard::ProcessController::Result KSysGuard::ProcessController::sendSignal(const QVariantList &pids, int signal)
{
    return sendSignal(d->listToVector(pids), signal);
}

ApplyResult KSysGuard::ProcessController::Private::applyToPids(const QList<int> &pids, const std::function<Processes::Error(int)> &function)
{
    ApplyResult result;

    for (auto pid : pids) {
        auto error = function(pid);
        switch (error) {
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

QList<int> KSysGuard::ProcessController::Private::listToVector(const QList<long long> &list)
{
    QList<int> vector;
    std::transform(list.cbegin(), list.cend(), std::back_inserter(vector), [](long long entry) {
        return entry;
    });
    return vector;
}

QList<int> KSysGuard::ProcessController::Private::listToVector(const QVariantList &list)
{
    QList<int> vector;
    std::transform(list.cbegin(), list.cend(), std::back_inserter(vector), [](const QVariant &entry) {
        return entry.toInt();
    });
    return vector;
}
