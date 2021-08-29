#ifndef GRAPHICSSIGNALPLOTTERBENCHMARK_H
#define GRAPHICSSIGNALPLOTTERBENCHMARK_H

#include <Qt>
#include <QtTest>

class KGraphicsSignalPlotter;
class QGraphicsView;
class QGraphicsScene;
class BenchmarkGraphicsSignalPlotter : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void addData();
    void addDataWhenHidden();

private:
    KGraphicsSignalPlotter *s;
    QGraphicsView *view;
    QGraphicsScene *scene;
};

#endif // GRAPHICSSIGNALPLOTTERBENCHMARK_H
