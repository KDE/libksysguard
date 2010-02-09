#include "graphicssignalplotterbenchmark.h"
#include "../../../libs/ksysguard/signalplotter/kgraphicssignalplotter.h"

#include <qtest_kde.h>
#include <QtTest>
#include <QGraphicsView>
#include <QGraphicsScene>
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
    s->resize(1000,500);
    view->resize(1010,510);
    view->show();
    s->setMaxAxisTextWidth(5);
    QTest::qWaitForWindowShown(view);

    QBENCHMARK {
        s->addSample(QList<qreal>() << qrand()%10 << qrand()%10 << qrand()%10 << qrand()%10);
        qApp->processEvents();
    }

}

void BenchmarkGraphicsSignalPlotter::addDataWhenHidden()
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

QTEST_KDEMAIN(BenchmarkGraphicsSignalPlotter, GUI)

