#include "signalplottertest.h"
#include "signalplotter/ksignalplotter.h"

#include <QtTestGui>
#include <QtGui>
#include <limits>

void TestSignalPlotter::init()
{
    s = new KSignalPlotter;
}
void TestSignalPlotter::cleanup()
{
    delete s;
}

void TestSignalPlotter::testAddRemoveBeams()
{
    //Just try various variations of adding and removing beams
    QCOMPARE(s->numBeams(), 0);

    s->addBeam(Qt::blue);
    s->addBeam(Qt::red);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);

    s->removeBeam(0);
    QCOMPARE(s->numBeams(), 1);
    QVERIFY(s->beamColor(0) == Qt::red);

    s->removeBeam(0);
    QCOMPARE(s->numBeams(), 0);

    s->addBeam(Qt::blue);
    s->addBeam(Qt::red);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);

    s->removeBeam(1);
    QCOMPARE(s->numBeams(), 1);
    QVERIFY(s->beamColor(0) == Qt::blue);

    s->addBeam(Qt::red);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);
}
void TestSignalPlotter::testAddRemoveBeamsWithData()
{
    //Just try various variations of adding and removing beams,
    //this time with data as well
    QCOMPARE(s->numBeams(), 0);

    s->addBeam(Qt::blue);
    s->addBeam(Qt::red);

    QVERIFY( std::isnan(s->lastValue(0)) ); //unset, so should default to NaN
    QVERIFY( std::isnan(s->lastValue(1)) ); //unset, so should default to NaN
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);
    s->addSample(QList<qreal>() << 1.0 << 2.0);
    QCOMPARE(s->lastValue(0), 1.0);
    QCOMPARE(s->lastValue(1), 2.0);

    s->removeBeam(0);
    QCOMPARE(s->numBeams(), 1);
    QVERIFY(s->beamColor(0) == Qt::red);
    QCOMPARE(s->lastValue(0), 2.0);

    s->removeBeam(0);
    QCOMPARE(s->numBeams(), 0);

    s->addBeam(Qt::blue);
    s->addBeam(Qt::red);
    s->addSample(QList<qreal>() << 1.0 << 2.0);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);
    QCOMPARE(s->lastValue(0), 1.0);
    QCOMPARE(s->lastValue(1), 2.0);

    s->removeBeam(1);
    QCOMPARE(s->numBeams(), 1);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QCOMPARE(s->lastValue(0), 1.0);

    s->addBeam(Qt::red);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);
    QCOMPARE(s->lastValue(0), 1.0);
    QVERIFY( std::isnan(s->lastValue(1)) ); //unset, so should default to NaN
}

void TestSignalPlotter::testReorderBeams()
{
    QCOMPARE(s->numBeams(), 0);
    QList<int> newOrder;
    s->reorderBeams(newOrder); // do nothing
    QCOMPARE(s->numBeams(), 0);

    s->addBeam(Qt::blue);
    s->addBeam(Qt::red);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);

    newOrder << 0 << 1; //nothing changed
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);

    newOrder.clear();
    newOrder << 1 << 0; //reverse them
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::red);
    QVERIFY(s->beamColor(1) == Qt::blue);

    //reverse them back again
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);

    //switch them yet again
    s->reorderBeams(newOrder);

    //Add a third beam
    s->addBeam(Qt::green);
    QCOMPARE(s->numBeams(), 3);
    QVERIFY(s->beamColor(0) == Qt::red);
    QVERIFY(s->beamColor(1) == Qt::blue);
    QVERIFY(s->beamColor(2) == Qt::green);

    newOrder.clear();
    newOrder << 2 << 0 << 1;
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 3);
    QVERIFY(s->beamColor(0) == Qt::green);
    QVERIFY(s->beamColor(1) == Qt::red);
    QVERIFY(s->beamColor(2) == Qt::blue);
}
void TestSignalPlotter::testReorderBeamsWithData()
{
    QCOMPARE(s->numBeams(), 0);
    QList<int> newOrder;

    s->addBeam(Qt::blue);
    s->addBeam(Qt::red);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(std::isnan(s->lastValue(0))); //unset, so should default to NaN
    QVERIFY(std::isnan(s->lastValue(1))); //unset, so should default to NaN
    //Add some data
    QList<qreal> data;
    data << 1.0 << 2.0;
    s->addSample(data);
    QCOMPARE(s->lastValue(0), 1.0);
    QCOMPARE(s->lastValue(1), 2.0);

    newOrder << 0 << 1; //nothing changed
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 2);
    QCOMPARE(s->lastValue(0), 1.0);
    QCOMPARE(s->lastValue(1), 2.0);

    newOrder.clear();
    newOrder << 1 << 0; //reverse them
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 2);
    QCOMPARE(s->lastValue(0), 2.0);
    QCOMPARE(s->lastValue(1), 1.0);

    //reverse them back again
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 2);
    QCOMPARE(s->lastValue(0), 1.0);
    QCOMPARE(s->lastValue(1), 2.0);

    //switch them yet again
    s->reorderBeams(newOrder);

    //Add a third beam
    s->addBeam(Qt::green);
    QCOMPARE(s->numBeams(), 3);
    QCOMPARE(s->lastValue(0), 2.0);
    QCOMPARE(s->lastValue(1), 1.0);
    QVERIFY(std::isnan(s->lastValue(2))); //unset, so should default to NaN

    newOrder.clear();
    newOrder << 2 << 0 << 1;
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 3);
    QVERIFY(std::isnan(s->lastValue(0))); //unset, so should default to NaN
    QCOMPARE(s->lastValue(1), 2.0);
    QCOMPARE(s->lastValue(2), 1.0);
}
void TestSignalPlotter::testMaximumRange()
{
    QCOMPARE(s->maximumValue(), 0.0);
    QCOMPARE(s->minimumValue(), 0.0);
    QCOMPARE(s->currentMaximumRangeValue(), 0.0);
    QCOMPARE(s->currentMinimumRangeValue(), 0.0);
    QCOMPARE(s->useAutoRange(), true);

    s->addBeam(Qt::blue);
    //Nothing should have changed yet
    QCOMPARE(s->maximumValue(), 0.0);
    QCOMPARE(s->minimumValue(), 0.0);
    QCOMPARE(s->currentMaximumRangeValue(), 0.0);
    QCOMPARE(s->currentMinimumRangeValue(), 0.0);

    QList<qreal> data;
    data << 1.1;
    s->addSample(data);

    QCOMPARE(s->maximumValue(), 0.0);
    QCOMPARE(s->minimumValue(), 0.0);

    QCOMPARE(s->currentMaximumRangeValue(), 1.1); //It gets rounded up.
    QCOMPARE(s->currentMinimumRangeValue(), 0.0);
    QCOMPARE(s->currentAxisPrecision(), 2); //step is 0.22

    s->setMaximumValue(1.0);
    QCOMPARE(s->maximumValue(), 1.0);
    QCOMPARE(s->minimumValue(), 0.0);
    QCOMPARE(s->currentMaximumRangeValue(), 1.1); //Current value is still larger
    QCOMPARE(s->currentMinimumRangeValue(), 0.0);
    QCOMPARE(s->currentAxisPrecision(), 2);

    s->setMaximumValue(1.4);
    QCOMPARE(s->maximumValue(), 1.4);
    QCOMPARE(s->minimumValue(), 0.0);
    QCOMPARE(s->currentMaximumRangeValue(), 1.4); //given maximum range is now the larger value
    QCOMPARE(s->currentMinimumRangeValue(), 0.0);
    QCOMPARE(s->currentAxisPrecision(), 2);


    s->addBeam(Qt::red);
    //nothing changed by adding a beam
    QCOMPARE(s->maximumValue(), 1.4);
    QCOMPARE(s->minimumValue(), 0.0);
    QCOMPARE(s->currentMaximumRangeValue(), 1.4); //given maximum range hasn't changed
    QCOMPARE(s->currentMinimumRangeValue(), 0.0);
    QCOMPARE(s->currentAxisPrecision(), 2);
}

void TestSignalPlotter::testNonZeroRange()
{
    s->addBeam(Qt::blue);
    s->setMinimumValue(10);
    s->setMaximumValue(20);

    QCOMPARE(s->currentMinimumRangeValue(), 10.0); //Current range should be 10, 12, 14, 16, 18, 20
    QCOMPARE(s->currentMaximumRangeValue(), 20.0);
    QCOMPARE(s->currentAxisPrecision(), 0);

    s->addSample(QList<qreal>() << 15);
    s->addSample(QList<qreal>() << 25);
    s->addSample(QList<qreal>() << 5);

    QCOMPARE(s->currentMinimumRangeValue(), 5.0);
    QCOMPARE(s->currentMaximumRangeValue(), 25.0); //Current range should be 5, 9, 13, 17, 21, 25
    QCOMPARE(s->currentAxisPrecision(), 0);

    s->addBeam(Qt::red);
    s->addSample(QList<qreal>() << 7 << 9);
    s->addSample(QList<qreal>() << 29.8 << 2);

    QCOMPARE(s->currentMinimumRangeValue(), 2.0);
    QCOMPARE(s->currentMaximumRangeValue(), 30.0); //Current range should be  2, 7.6, 13.2, 18.8, 24.4, 30
    QCOMPARE(s->currentAxisPrecision(), 1);

    s->addSample(QList<qreal>() << std::numeric_limits<qreal>::quiet_NaN()); //These should appear as gaps in the data

    QCOMPARE(s->currentMinimumRangeValue(), 2.0);
    QCOMPARE(s->currentMaximumRangeValue(), 30.0);
    QCOMPARE(s->currentAxisPrecision(), 1);

    s->addSample(QList<qreal>() << 1.0/0.0 << -1.0/0.0);

    QCOMPARE(s->currentMinimumRangeValue(), 2.0);
    QCOMPARE(s->currentMaximumRangeValue(), 30.0);
    QCOMPARE(s->currentAxisPrecision(), 1);
}

void TestSignalPlotter::testNonZeroRange2()
{
    s->addBeam(Qt::blue);
    s->setMinimumValue(22);
    s->setMaximumValue(23);

    QCOMPARE(s->currentMinimumRangeValue(), 22.0);
    QCOMPARE(s->currentMaximumRangeValue(), 23.0);

    s->addSample(QList<qreal>() << 25);
    QCOMPARE(s->currentMinimumRangeValue(), 22.0);
    QCOMPARE(s->currentMaximumRangeValue(), 25.0);

}

void TestSignalPlotter::testNiceRangeCalculation_data()
{
    QTest::addColumn<qreal>("min");
    QTest::addColumn<qreal>("max");
    QTest::addColumn<qreal>("niceMin");
    QTest::addColumn<qreal>("niceMax");
    QTest::addColumn<int>("precision");

#define STRINGIZE(number) #number
#define testRange(min,max,niceMin,niceMax, precision) QTest::newRow(STRINGIZE(min) " to " STRINGIZE(max)) << qreal(min) << qreal(max) << qreal(niceMin) << qreal(niceMax) << int(precision)
/*    testRange(-49,  199, -50, 200, 0);      // Scale should read -50,   0,   50,  100, 150,  200
    testRange(-50,  199, -50, 200, 0);      // Scale should read -50,   0,   50,  100, 150,  200
    testRange(-49,  200, -50, 200, 0);      // Scale should read -50,   0,   50,  100, 150,  200
    testRange(-50,  200, -50, 200, 0);      // Scale should read -50,   0,   50,  100, 150,  200
    testRange(-1,   199, -50, 200, 0);      // Scale should read -50,   0,   50,  100, 150,  200
    testRange(-99,  149, -100, 150, 0);     // Scale should read -100,  50,  0,   50,  100,  150
    testRange(-100, 150, -100, 150, 0);     // Scale should read -100,  50,  0,   50,  100,  150
    testRange(-1000, 1000, -1000, 1500, 0); // Scale should read -1000, 500, 0,   500, 1000, 1500 */
    testRange(0, 7, 0, 7, 1);               // Scale should read 0,     1.4, 2.8, 4.2, 5.6,  7
}
void TestSignalPlotter::testNiceRangeCalculation()
{
    QFETCH(qreal, min);
    QFETCH(qreal, max);
    QFETCH(qreal, niceMin);
    QFETCH(qreal, niceMax);
    QFETCH(int, precision);

    s->addBeam(Qt::blue);
    s->changeRange(min, max);

    QCOMPARE(s->currentMinimumRangeValue(), niceMin);
    QCOMPARE(s->currentMaximumRangeValue(), niceMax);
    QCOMPARE(s->currentAxisPrecision(), precision);
}

void TestSignalPlotter::testNegativeMinimumRange()
{
    s->setMinimumValue(-1000);
    s->setMaximumValue(4000);
    QCOMPARE(s->minimumValue(), -1000.0);
    QCOMPARE(s->maximumValue(),  4000.0);
    QCOMPARE(s->currentMinimumRangeValue(), -1000.0);
    QCOMPARE(s->currentMaximumRangeValue(), 4000.0);

    s->setScaleDownBy(1024);
    QCOMPARE(s->minimumValue(), -1000.0);
    QCOMPARE(s->maximumValue(),  4000.0);
    QCOMPARE(s->currentMaximumRangeValue(), 4014.08); //The given range was -0.976KB to 3.906KB.  This was rounded as: -0.98KB to  3.92KB
    QCOMPARE(s->currentMinimumRangeValue(), -1003.52);

    QCOMPARE(s->valueAsString(4096,1), QString("4.0"));
    QCOMPARE(s->valueAsString(-4096,1), QString("-4.0"));

    s->addBeam(Qt::red);
    s->addSample(QList<qreal>() << -1024.0);
    QCOMPARE(s->currentMaximumRangeValue(), 4096.0);
    QCOMPARE(s->currentMinimumRangeValue(), -1024.0);
    s->addSample(QList<qreal>() << -1025.0); //Scale now becomes  -3, -1.5, 0, 1.5, 3, 4.5 in KB
    QCOMPARE(s->currentMinimumRangeValue(), -1126.4); //-1.1KB
    QCOMPARE(s->currentMaximumRangeValue(), 4505.6); //4.4KB
}
void TestSignalPlotter::testSetBeamColor() {
    s->addBeam(Qt::red);
    s->setBeamColor(0, Qt::blue);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QCOMPARE(s->numBeams(), 1);

    s->addBeam(Qt::red);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);
    QCOMPARE(s->numBeams(), 2);

    s->setBeamColor(0, Qt::green);
    QVERIFY(s->beamColor(0) == Qt::green);
    QVERIFY(s->beamColor(1) == Qt::red);
    QCOMPARE(s->numBeams(), 2);

    s->setBeamColor(1, Qt::blue);
    QVERIFY(s->beamColor(0) == Qt::green);
    QVERIFY(s->beamColor(1) == Qt::blue);

    s->removeBeam(0);
    QVERIFY(s->beamColor(0) == Qt::blue);
    s->setBeamColor(0, Qt::red);
    QVERIFY(s->beamColor(0) == Qt::red);
}

void TestSignalPlotter::testSetUnit() {
    //Test default
    QCOMPARE(s->valueAsString(3e20,1), QString("3e+20"));
    QCOMPARE(s->valueAsString(-3e20,1), QString("-3e+20"));

    s->setUnit(ki18ncp("Units", "%1 second", "%1 seconds") );

    QSKIP("I18n problems");
    QCOMPARE(s->valueAsString(3e20,1), QString("3e+20 seconds"));
    QCOMPARE(s->valueAsString(-3e20,1), QString("-3e+20 seconds"));
    QCOMPARE(s->valueAsString(3.4,1), QString("3.4 seconds"));
    QCOMPARE(s->valueAsString(-3.4,1), QString("-3.4 seconds"));
    QCOMPARE(s->valueAsString(1), QString("1.0 seconds"));
    QCOMPARE(s->valueAsString(-1), QString("-1.0 seconds"));
    QCOMPARE(s->valueAsString(1,0), QString("1 second"));
    QCOMPARE(s->valueAsString(-1,0), QString("-1 second"));

    //now switch to minutes
    s->setScaleDownBy(60);
    s->setUnit(ki18ncp("Units", "%1 minute", "%1 minutes") );
    QCOMPARE(s->valueAsString(3.4), QString("0.06 minutes"));
    QCOMPARE(s->valueAsString(-3.4), QString("-0.06 minutes"));
    QCOMPARE(s->valueAsString(60), QString("1.0 minutes"));
    QCOMPARE(s->valueAsString(-60), QString("-1.0 minutes"));
    QCOMPARE(s->valueAsString(60,0), QString("1 minute"));
    QCOMPARE(s->valueAsString(-60,0), QString("-1 minute"));
}

void TestSignalPlotter::testGettersSetters() {
    //basic test of all the getters and setters and default values
    KLocalizedString string = ki18ncp("Units", "%1 second", "%1 seconds");
    s->setUnit( string );
    QVERIFY( s->unit().toString() == string.toString() );
    s->setMaximumValue(3);
    s->setMinimumValue(-3);
    QCOMPARE(s->maximumValue(), 3.0);
    QCOMPARE(s->minimumValue(), -3.0);

    s->changeRange(-2,2);
    QCOMPARE(s->maximumValue(), 2.0);
    QCOMPARE(s->minimumValue(), -2.0);

    s->setMinimumValue(-3);
    QCOMPARE(s->useAutoRange(), true); //default
    s->setUseAutoRange(false);
    QCOMPARE(s->useAutoRange(), false);

    QCOMPARE(s->scaleDownBy(), 1.0); //default
    s->setScaleDownBy(1.2);
    QCOMPARE(s->scaleDownBy(), 1.2);
    s->setScaleDownBy(0.5);
    QCOMPARE(s->scaleDownBy(), 0.5);

    QCOMPARE(s->horizontalScale(), 6); //default
    s->setHorizontalScale(2);
    QCOMPARE(s->horizontalScale(), 2);
    s->setHorizontalScale(1);
    QCOMPARE(s->horizontalScale(), 1);
    s->setHorizontalScale(0); // Ignored - invalid value
    QCOMPARE(s->horizontalScale(), 1);

    QCOMPARE(s->showHorizontalLines(), true); //default
    s->setShowHorizontalLines(false);
    QCOMPARE(s->showHorizontalLines(), false);

    QCOMPARE(s->showVerticalLines(), false); //default
    s->setShowVerticalLines(true);
    QCOMPARE(s->showVerticalLines(), true);

    QCOMPARE(s->verticalLinesScroll(), true); //default
    s->setVerticalLinesScroll(false);
    QCOMPARE(s->verticalLinesScroll(), false);

    QCOMPARE(s->verticalLinesDistance(), (uint)30); //default
    s->setVerticalLinesDistance(1);
    QCOMPARE(s->verticalLinesDistance(), (uint)1);

    QCOMPARE(s->showAxis(), true); //default
    s->setShowAxis(false);
    QCOMPARE(s->showAxis(), false);

    QCOMPARE(s->maxAxisTextWidth(), 0); //default
    s->setMaxAxisTextWidth(30);
    QCOMPARE(s->maxAxisTextWidth(), 30);
    s->setMaxAxisTextWidth(0);
    QCOMPARE(s->maxAxisTextWidth(), 0);

    QCOMPARE(s->smoothGraph(), true); //default
    s->setSmoothGraph(false);
    QCOMPARE(s->smoothGraph(), false);

    QCOMPARE(s->stackGraph(), false); //default
    s->setStackGraph(true);
    QCOMPARE(s->stackGraph(), true);

    QCOMPARE(s->fillOpacity(), 20); //default
    s->setFillOpacity(255);
    QCOMPARE(s->fillOpacity(), 255);
    s->setFillOpacity(0);
    QCOMPARE(s->fillOpacity(), 0);


}
void TestSignalPlotter::testAddingData()
{
    QCOMPARE(s->useAutoRange(), true);
    s->setGeometry(0,0,500,500);
    //Test adding sample without any beams.  It should just ignore this
    s->addSample(QList<qreal>() << 1.0 << 2.0);
    //Test setting the beam color of a non-existant beam.  It should just ignore this too.
//    s->setBeamColor(0, Qt::blue);

    //Add an empty sample.  This should just be ignored?
    s->addSample(QList<qreal>());

    //Okay let's be serious now
    s->addBeam(Qt::red);
    s->addSample(QList<qreal>() << 0.0);
    s->addSample(QList<qreal>() << -0.0);
    s->addSample(QList<qreal>() << -1.0);
    s->addSample(QList<qreal>() << -1000.0);
    s->addSample(QList<qreal>() << 1000.0);
    s->addSample(QList<qreal>() << 300.0);
    s->addSample(QList<qreal>() << 300.0);
    s->addSample(QList<qreal>() << 300.0);
    s->addSample(QList<qreal>() << 300.0);
    s->addSample(QList<qreal>() << 300.0);
    s->addSample(QList<qreal>() << 300.0);
    s->addSample(QList<qreal>() << 1.0/0.0); //Positive infinity.  Should be ignore for range values, not crash, and draw something reasonable
    s->addSample(QList<qreal>() << 1.0/0.0); //Positive infinity.  Should be ignore for range values, not crash, and draw something reasonable
    s->addSample(QList<qreal>() << 1.0/0.0); //Positive infinity.  Should be ignore for range values, not crash, and draw something reasonable
    s->addSample(QList<qreal>() << 1.0/0.0); //Positive infinity.  Should be ignore for range values, not crash, and draw something reasonable
    s->addSample(QList<qreal>() << 1.0/0.0); //Positive infinity.  Should be ignore for range values, not crash, and draw something reasonable
    s->addSample(QList<qreal>() << -1.0/0.0); //Positive infinity.  Likewise.
    s->addSample(QList<qreal>() << -1.0/0.0); //Negative infinity.  Likewise.
    s->addSample(QList<qreal>() << -1.0/0.0); //Negative infinity.  Likewise.
    s->addSample(QList<qreal>() << -1.0/0.0); //Negative infinity.  Likewise.
    s->addSample(QList<qreal>() << -1.0/0.0); //Negative infinity.  Likewise.
    s->addSample(QList<qreal>() << -1.0/0.0); //Negative infinity.  Likewise.
    s->addSample(QList<qreal>() << 300.0);
    s->addSample(QList<qreal>() << 300.0);
    s->addSample(QList<qreal>() << 300.0);
    s->addSample(QList<qreal>() << 300.0);
    s->addSample(QList<qreal>() << std::numeric_limits<qreal>::quiet_NaN()); //These should appear as gaps in the data
    s->addSample(QList<qreal>() << 300.0);
    s->addSample(QList<qreal>() << 300.0);
    s->addSample(QList<qreal>() << std::numeric_limits<qreal>::quiet_NaN());
    s->addSample(QList<qreal>() << 400.0);
    s->addSample(QList<qreal>() << std::numeric_limits<qreal>::quiet_NaN());
    s->addSample(QList<qreal>() << std::numeric_limits<qreal>::quiet_NaN());
    s->addBeam(Qt::green);
    s->addSample(QList<qreal>() << std::numeric_limits<qreal>::quiet_NaN() << 100.0);
    s->addSample(QList<qreal>() << std::numeric_limits<qreal>::quiet_NaN() << 100.0);
    s->addSample(QList<qreal>() << 200.0 << 100.0);
    s->addSample(QList<qreal>() << 300.0 << 100.0);
    s->addBeam(Qt::blue);
    s->addSample(QList<qreal>() << 400.0 << 100.0 << 200.0);
    s->addSample(QList<qreal>() << 500.0 << 100.0 << 200.0);
    s->addSample(QList<qreal>() << 600.0 << 100.0 << 200.0);

    QCOMPARE(s->currentMinimumRangeValue(), -1000.0);
    QCOMPARE(s->currentMaximumRangeValue(), 1500.0);

    //Paint to a device, to check that the painter does not crash etc
    QPixmap pixmap(s->size());
    s->render(&pixmap);

    // For debugging, show the widget so that we can check it visually
//    s->show();
//    QTest::qWait(10000);

    //Test that it does not crash at small sizes
    for(int x = 0; x < 4; x++)
        for(int y = 0; y < 4; y++) {
            s->setGeometry(0,0,x,y);
            s->render(&pixmap);
        }
}
QTEST_MAIN(TestSignalPlotter)

