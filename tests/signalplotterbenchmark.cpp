#include "signalplotterbenchmark.h"
#include "signalplotter/ksignalplotter.h"

#include <QtTestGui>
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
    s->setMaxAxisTextWidth(5);
    s->resize(1000,500);
    QTest::qWaitForWindowExposed(s);

    QBENCHMARK {
        s->addSample(QList<qreal>() << qrand()%10 << qrand()%10 << qrand()%10 << qrand()%10);
        qApp->processEvents();
    }

}
void BenchmarkSignalPlotter::stackedData()
{
    s->addBeam(Qt::blue);
    s->addBeam(Qt::green);
    s->addBeam(Qt::red);
    s->addBeam(Qt::yellow);
    s->setStackGraph(true);
    s->show();
    s->setMaxAxisTextWidth(5);
    s->resize(1000,500);
    QTest::qWaitForWindowExposed(s);

    QBENCHMARK {
        s->addSample(QList<qreal>() << qrand()%10 << qrand()%10 << qrand()%10 << qrand()%10);
        qApp->processEvents();
    }

}
void BenchmarkSignalPlotter::addDataWhenHidden()
{
    s->addBeam(Qt::blue);
    s->addBeam(Qt::green);
    s->addBeam(Qt::red);
    s->addBeam(Qt::yellow);

    QBENCHMARK {
        s->addSample(QList<qreal>() << qrand()%10 << qrand()%10 << qrand()%10 << qrand()%10);
        qApp->processEvents();
    }

}

QTEST_MAIN(BenchmarkSignalPlotter)

