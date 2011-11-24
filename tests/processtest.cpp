/*  This file is part of the KDE project
    Copyright (C) 2007 John Tapsell <tapsell@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include <QtTest>
#include <QtCore>
#include <QTreeView>

#include <klocale.h>
#include <qtest_kde.h>
#include <kdebug.h>

#include "processcore/processes.h"
#include "processcore/process.h"
#include "processcore/processes_base_p.h"

#include "processui/ksysguardprocesslist.h"

#include "processtest.h"

void testProcess::testProcesses() {
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    processController->updateAllProcesses();
    QList<KSysGuard::Process *> processes = processController->getAllProcesses();
    QSet<long> pids;
    Q_FOREACH( KSysGuard::Process *process, processes) {
        if(process->pid == 0) continue;
        QVERIFY(process->pid > 0);
        QVERIFY(!process->name.isEmpty());

        //test all the pids are unique
        QVERIFY(!pids.contains(process->pid));
        pids.insert(process->pid);
    }
    processController->updateAllProcesses();
    QList<KSysGuard::Process *> processes2 = processController->getAllProcesses();
    Q_FOREACH( KSysGuard::Process *process, processes2) {
        if(process->pid == 0) continue;
        QVERIFY(process->pid > 0);
        QVERIFY(!process->name.isEmpty());

        //test all the pids are unique
        if(!pids.contains(process->pid)) {
            kDebug() << process->pid << " not found. " << process->name;
        }
        pids.remove(process->pid);
    }

    QVERIFY(processes2.size() == processes.size());
    QCOMPARE(processes, processes2); //Make sure calling it twice gives the same results.  The difference in time is so small that it really shouldn't have changed
    delete processController;
}


unsigned long testProcess::countNumChildren(KSysGuard::Process *p) {
    unsigned long total = p->children.size();
    for(int i =0; i < p->children.size(); i++) {
        total += countNumChildren(p->children[i]);
    }
    return total;
}

void testProcess::testProcessesTreeStructure() {
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    processController->updateAllProcesses();
    QList<KSysGuard::Process *> processes = processController->getAllProcesses();
    
    Q_FOREACH( KSysGuard::Process *process, processes) {
        QCOMPARE(countNumChildren(process), process->numChildren);

        for(int i =0; i < process->children.size(); i++) {
            QVERIFY(process->children[i]->parent);
            QCOMPARE(process->children[i]->parent, process);
        }
    }
    delete processController;
}

void testProcess::testProcessesModification() {
    //We will modify the tree, then re-call getProcesses and make sure that it fixed everything we modified
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    processController->updateAllProcesses();
    KSysGuard::Process *initProcess = processController->getProcess(1);

    if(!initProcess || initProcess->numChildren < 3) {
        delete processController;
        return;
    }

    QVERIFY(initProcess);
    QVERIFY(initProcess->children[0]);
    QVERIFY(initProcess->children[1]);
    kDebug() << initProcess->numChildren;
    initProcess->children[0]->parent = initProcess->children[1];
    initProcess->children[1]->children.append(initProcess->children[0]);
    initProcess->children[1]->numChildren++;
    initProcess->numChildren--;
    initProcess->children.removeAt(0);
    delete processController;
}

void testProcess::testTimeToUpdateAllProcesses() {
    //See how long it takes to get process information
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    QBENCHMARK {
        processController->updateAllProcesses();
    }
}
void testProcess::testTimeToUpdateModel() {
    KSysGuardProcessList *processList = new KSysGuardProcessList;
    processList->treeView()->setColumnHidden(13, false);
    processList->show();
    QTest::qWaitForWindowShown(processList);

    QBENCHMARK {
        processList->updateList();
        QTest::qWait(0);
    }
    delete processList;
}

void testProcess::testHistories() {
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    QBENCHMARK_ONCE {
        if(!processController->isHistoryAvailable()) {
            qWarning("History was not available");
            delete processController;
            return;
        }
    }
    QCOMPARE(processController->historyFileName(), QString("/var/log/atop.log"));
    QList< QPair<QDateTime, uint> > history = processController->historiesAvailable();
    bool success = processController->setViewingTime(history[0].first);
    QVERIFY(success);
    QVERIFY(processController->viewingTime() == history[0].first);
    success = processController->setViewingTime(history[0].first.addSecs(-1));
    QVERIFY(success);
    QVERIFY(processController->viewingTime() == history[0].first);
    success = processController->setViewingTime(history[0].first.addSecs(-history[0].second -1));
    QVERIFY(!success);
    QVERIFY(processController->viewingTime() == history[0].first);
    QCOMPARE(processController->historyFileName(), QString("/var/log/atop.log"));
    
    //Test the tree structure
    processController->updateAllProcesses();
    QList<KSysGuard::Process *> processes = processController->getAllProcesses();
    
    Q_FOREACH( KSysGuard::Process *process, processes) {
        QCOMPARE(countNumChildren(process), process->numChildren);

        for(int i =0; i < process->children.size(); i++) {
            QVERIFY(process->children[i]->parent);
            QCOMPARE(process->children[i]->parent, process);
        }
    }

    //test all the pids are unique
    QSet<long> pids;
    Q_FOREACH( KSysGuard::Process *process, processes) {
        if(process->pid == 0) continue;
        QVERIFY(process->pid > 0);
        QVERIFY(!process->name.isEmpty());

        QVERIFY(!pids.contains(process->pid));
        pids.insert(process->pid);
    }
    delete processController;
}

void testProcess::testUpdateOrAddProcess() {
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    processController->updateAllProcesses();
    KSysGuard::Process *process;
    // Make sure that this doesn't crash at least
    process = processController->getProcess(0);
    process = processController->getProcess(1);
    if (process)
        QCOMPARE(process->pid, 1l);

    // Make sure that this doesn't crash at least
    processController->updateOrAddProcess(1);
    processController->updateOrAddProcess(0);
    processController->updateOrAddProcess(-1);
}
void testProcess::testHistoriesWithWidget() {
    KSysGuardProcessList *processList = new KSysGuardProcessList;
    processList->treeView()->setColumnHidden(13, false);
    processList->show();
    QTest::qWaitForWindowShown(processList);
    KSysGuard::Processes *processController = processList->processModel()->processController();
        
    QList< QPair<QDateTime, uint> > history = processController->historiesAvailable();

    for(int i = 0; i < history.size(); i++) {
        qDebug() << "Viewing time" << history[i].first;
        bool success = processController->setViewingTime(history[i].first);
        QVERIFY(success);
        QCOMPARE(processController->viewingTime(), history[i].first);
        processList->updateList();
        QTest::qWait(100);
    }
    delete processList;
}
QTEST_KDEMAIN(testProcess,GUI)

#include "processtest.moc"

