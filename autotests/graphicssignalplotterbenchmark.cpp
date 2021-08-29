#include "graphicssignalplotterbenchmark.h"
#include "signalplotter/kgraphicssignalplotter.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QRandomGenerator>
#include <QtTestGui>
#include <limits>

void BenchmarkGraphicsSignalPlotter::init()
{
    scene = new QGraphicsScene;
    view = new QGraphicsView(scene);
    s = new KGraphicsSignalPlotter;
    scene->addItem(s);
}
void BenchmarkGraphicsSignalPlotter::cleanup()
{
    delete view;
    delete scene;
}

void BenchmarkGraphicsSignalPlotter::addData()
{
    s->addBeam(Qt::blue);
    s->addBeam(Qt::green);
    s->addBeam(Qt::red);
    s->addBeam(Qt::yellow);
    s->resize(1000, 500);
    view->resize(1010, 510);
    view->show();
    s->setMaxAxisTextWidth(5);
    QVERIFY(QTest::qWaitForWindowExposed(view));

    auto *generator = QRandomGenerator::global();

    QBENCHMARK {
        s->addSample(QList<qreal>() << generator->bounded(10) << generator->bounded(10) << generator->bounded(10) << generator->bounded(10));
        qApp->processEvents();
    }
}

void BenchmarkGraphicsSignalPlotter::addDataWhenHidden()
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

QTEST_MAIN(BenchmarkGraphicsSignalPlotter)
