#include "signalplotterbenchmark.h"
#include "signalplotter/ksignalplotter.h"

#include <QRandomGenerator>
#include <QtGui>
#include <QtTestGui>
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
    s->resize(1000, 500);
    QVERIFY(QTest::qWaitForWindowExposed(s));

    auto *generator = QRandomGenerator::global();
    QBENCHMARK {
        s->addSample(QList<qreal>() << generator->bounded(10) << generator->bounded(10) << generator->bounded(10) << generator->bounded(10));
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
    s->resize(1000, 500);
    QVERIFY(QTest::qWaitForWindowExposed(s));

    auto *generator = QRandomGenerator::global();
    QBENCHMARK {
        s->addSample(QList<qreal>() << generator->bounded(10) << generator->bounded(10) << generator->bounded(10) << generator->bounded(10));
        qApp->processEvents();
    }
}
void BenchmarkSignalPlotter::addDataWhenHidden()
{
    s->addBeam(Qt::blue);
    s->addBeam(Qt::green);
    s->addBeam(Qt::red);
    s->addBeam(Qt::yellow);

    auto *generator = QRandomGenerator::global();
    QBENCHMARK {
        s->addSample(QList<qreal>() << generator->bounded(10) << generator->bounded(10) << generator->bounded(10) << generator->bounded(10));
        qApp->processEvents();
    }
}

QTEST_MAIN(BenchmarkSignalPlotter)
