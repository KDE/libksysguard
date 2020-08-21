/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QTest>
#include <QThreadPool>

#include <unistd.h>

#define private public
#include "ReadProcStatusTask.h"
#undef private

#include "processcore/process.h"

class ProcStatusTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test()
    {
        auto pid = getpid();
        auto task = new KSysGuard::ReadProcStatusTask(QStringLiteral("/proc/%1").arg(pid));

        QThreadPool::globalInstance()->start(task);

        QThreadPool::globalInstance()->waitForDone();

        QVERIFY(task->isFinished());
        QVERIFY(task->isSuccessful());

        QCOMPARE(task->m_processName, QStringLiteral("procstatustest"));

        QCOMPARE(task->m_uid, getuid());
        QCOMPARE(task->m_euid, geteuid());

        QCOMPARE(task->m_gid, getgid());
        QCOMPARE(task->m_egid, getegid());

        KSysGuard::Process process{pid, -1, nullptr};
        task->updateProcess(&process);

        QCOMPARE(process.uid(), getuid());
        QCOMPARE(process.gid(), getgid());

        delete task;
    }
};

QTEST_MAIN(ProcStatusTest);

#include "procstatustest.moc"
