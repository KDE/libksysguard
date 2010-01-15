#include "signalplotterbenchmark.h"
#include "../../../libs/ksysguard/signalplotter/ksignalplotter.h"

#include <qtest_kde.h>
#include <QtTest>
#include <QtGui>
#include <limits>

void BenchmarkSignalPlotter::init()
{
    s = new KSignalPlotter;
}
void BenchmarkSignalPlotter::cleanup()
{
    delete s;
}

void BenchmarkSignalPlotter::addData()
{
    s->addBeam(Qt::blue);
    s->addBeam(Qt::green);
    s->addBeam(Qt::red);
    s->addBeam(Qt::yellow);
    s->show();
    s->resize(1000,500);
    QTest::qWaitForWindowShown(s);

    QBENCHMARK {
        s->addSample(QList<double>() << qrand()%10 << qrand()%10 << qrand()%10 << qrand()%10);
        s->repaint();
    }

}
QTEST_KDEMAIN(BenchmarkSignalPlotter, GUI)

