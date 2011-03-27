
#include <QtTest>
#include <Qt>

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
