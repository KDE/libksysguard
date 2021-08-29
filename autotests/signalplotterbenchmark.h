#ifndef KSYSGUARD_SIGNALPLOTTERBENCHMARK_H
#define KSYSGUARD_SIGNALPLOTTERBENCHMARK_H

#include <Qt>
#include <QtTest>

class KSignalPlotter;
class BenchmarkSignalPlotter : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void addData();
    void stackedData();
    void addDataWhenHidden();

private:
    KSignalPlotter *s;
};

#endif // KSYSGUARD_SIGNALPLOTTERBENCHMARK_H
