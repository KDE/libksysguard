/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QDebug>
#include <QProcess>
#include <QtCore>
#include <QtTestGui>

#include <limits>

#include "processcore/process.h"
#include "processcore/processes.h"
#include "processcore_debug.h"

#include "processtest.h"

void testProcess::testProcesses()
{
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    processController->updateAllProcesses();
    const QList<KSysGuard::Process *> processes = processController->getAllProcesses();
    QSet<long> pids;
    for (KSysGuard::Process *process : processes) {
        if (process->pid() == 0)
            continue;
        QVERIFY(process->pid() > 0);
        QVERIFY(!process->name().isEmpty());

        // test all the pids are unique
        QVERIFY(!pids.contains(process->pid()));
        pids.insert(process->pid());
    }
    processController->updateAllProcesses();
    const QList<KSysGuard::Process *> processes2 = processController->getAllProcesses();
    for (KSysGuard::Process *process : processes2) {
        if (process->pid() == 0)
            continue;
        QVERIFY(process->pid() > 0);
        QVERIFY(!process->name().isEmpty());

        // test all the pids are unique
        if (!pids.contains(process->pid())) {
            qCDebug(LIBKSYSGUARD_PROCESSCORE) << process->pid() << " not found. " << process->name();
        }
        pids.remove(process->pid());
    }

    QVERIFY(processes2.size() == processes.size());
    QCOMPARE(processes,
             processes2); // Make sure calling it twice gives the same results.  The difference in time is so small that it really shouldn't have changed
    delete processController;
}

unsigned long testProcess::countNumChildren(KSysGuard::Process *p)
{
    unsigned long total = p->children().size();
    for (int i = 0; i < p->children().size(); i++) {
        total += countNumChildren(p->children()[i]);
    }
    return total;
}

void testProcess::testProcessesTreeStructure()
{
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    processController->updateAllProcesses();

    auto verify_counts = [this](const auto processes) {
        for (KSysGuard::Process *process : processes) {
            QCOMPARE(countNumChildren(process), process->numChildren());

            for(int i = 0; i < process->children().size(); i++) {
                QVERIFY(process->children()[i]->parent());
                QCOMPARE(process->children()[i]->parent(), process);
            }
        }
    };

    verify_counts(processController->getAllProcesses());

    // this should test if the children accounting isn't off on updates
    QProcess proc;
    proc.start(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), QStringLiteral("sleep 100& (sleep 50; sleep 50) & while true; do :; done")});
    QVERIFY(proc.waitForStarted());
    QTest::qSleep(2000);

    processController->updateAllProcesses();
    verify_counts(processController->getAllProcesses());

    proc.terminate();

    QVERIFY(proc.waitForFinished());
    processController->updateAllProcesses();
    verify_counts(processController->getAllProcesses());

    delete processController;
}

void testProcess::testProcessesModification()
{
    // We will modify the tree, then re-call getProcesses and make sure that it fixed everything we modified
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    processController->updateAllProcesses();
    KSysGuard::Process *initProcess = processController->getProcess(1);

    if (!initProcess || initProcess->numChildren() < 3) {
        delete processController;
        return;
    }

    QVERIFY(initProcess);
    QVERIFY(initProcess->children()[0]);
    QVERIFY(initProcess->children()[1]);
    qCDebug(LIBKSYSGUARD_PROCESSCORE) << initProcess->numChildren();
    initProcess->children()[0]->setParent(initProcess->children()[1]);
    initProcess->children()[1]->children().append(initProcess->children()[0]);
    initProcess->children()[1]->numChildren()++;
    initProcess->numChildren()--;
    initProcess->children().removeAt(0);
    delete processController;
}

void testProcess::testTimeToUpdateAllProcesses()
{
    // See how long it takes to get process information
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    QBENCHMARK {
        processController->updateAllProcesses();
    }
    delete processController;
}

void testProcess::testUpdateOrAddProcess()
{
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    processController->updateAllProcesses();
    KSysGuard::Process *process;
    // Make sure that this doesn't crash at least
    processController->getProcess(0);
    process = processController->getProcess(1);
    if (process)
        QCOMPARE(process->pid(), 1l);

    // Make sure that this doesn't crash at least
    processController->updateOrAddProcess(1);
    processController->updateOrAddProcess(0);
    processController->updateOrAddProcess(-1);

    processController->updateOrAddProcess(std::numeric_limits<long>::max()-1);
    QVERIFY(processController->getProcess(std::numeric_limits<long>::max()-1));
    processController->updateAllProcesses();
    QVERIFY(processController->getProcess(std::numeric_limits<long>::max()-1));
    QCOMPARE(processController->getProcess(std::numeric_limits<long>::max()-1)->status(), KSysGuard::Process::Ended);
    processController->updateAllProcesses();
    QVERIFY(!processController->getProcess(std::numeric_limits<long>::max()-1));
}

QTEST_MAIN(testProcess)
