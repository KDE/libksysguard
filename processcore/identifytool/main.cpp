
#include <iostream>

#include <QCommandLineParser>
#include <QCoreApplication>

#include <KUser>

#include "../cgroup.h"
#include "../processes.h"

using namespace Qt::StringLiterals;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Helper tool to identify which application a process is part of."_s);
    parser.addHelpOption();
    parser.addPositionalArgument(u"pid"_s, u"The ID of the process to identify."_s);
    parser.addOptions({
        {"all", "Try to identify all user processes."},
    });
    parser.process(app);

    if (parser.positionalArguments().isEmpty() && !parser.isSet(u"all"_s)) {
        parser.showHelp(1);
    }

    auto pid = parser.positionalArguments().value(0).toInt();

    KSysGuard::Processes processes;
    processes.updateAllProcesses();

    auto userId = KUserId::currentEffectiveUserId();

    const auto allProcesses = processes.getAllProcesses();
    for (auto process : allProcesses) {
        if (parser.isSet(u"all"_s)) {
            if (process->euid() != userId.nativeId()) {
                continue;
            }
        } else if (process->pid() != pid) {
            continue;
        }

        std::cout << "Process " << process->pid() << ":\n";

        std::cout << "  name: " << process->name().toStdString() << "\n";
        std::cout << "  command: " << process->command().toStdString() << "\n";
        std::cout << "\n";

        KSysGuard::CGroup cgroup(process->cGroup());
        std::cout << "  cgroup: " << cgroup.id().toStdString() << "\n";
        std::cout << "  guessed application id: " << cgroup.appId().toStdString() << "\n";
        if (!cgroup.service()->menuId().isEmpty()) {
            std::cout << "  actual application id: " << cgroup.service()->menuId().toStdString() << "\n";
            std::cout << "  application name: " << cgroup.service()->name().toStdString() << "\n";
        } else {
            std::cout << "  application: Unknown\n";
        }

        std::cout << std::endl;
    }

    return 0;
}
