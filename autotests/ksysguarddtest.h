#ifndef KSYSGUARD_KSYSGUARDDTEST_H
#define KSYSGUARD_KSYSGUARDDTEST_H

#include <Qt>
#include <QtTest>

#include "ksgrd/SensorAgent.h"
#include "ksgrd/SensorClient.h"
#include "ksgrd/SensorManager.h"
#include <QDebug>
#include <QObject>
#include <QProcess>
class SensorClientTest;

class TestKsysguardd : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testSetup();
    void testFormatting_data();
    void testFormatting();
    void testQueueing();

private:
    KSGRD::SensorManager manager;
    SensorClientTest *client;
    QSignalSpy *hostConnectionLostSpy;
    QSignalSpy *updateSpy;
    QSignalSpy *hostAddedSpy;
    int nextId;
};
struct Answer {
    Answer()
    {
        id = -1;
        isSensorLost = false;
    }
    int id;
    QList<QByteArray> answer;
    bool isSensorLost;
};
struct SensorClientTest : public KSGRD::SensorClient {
    SensorClientTest()
    {
        isSensorLost = false;
        haveAnswer = false;
    }
    virtual void answerReceived(int id, const QList<QByteArray> &answer_)
    {
        Answer answer;
        answer.id = id;
        answer.answer = answer_;
        answers << answer;
        haveAnswer = true;
    }
    virtual void sensorLost(int id)
    {
        Answer answer;
        answer.id = id;
        answer.isSensorLost = true;
        answers << answer;
        isSensorLost = true;
    }
    bool isSensorLost;
    bool haveAnswer;
    QList<Answer> answers;
};

#endif // KSYSGUARD_KSYSGUARDDTEST_H
