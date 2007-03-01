#include <QtTest>
#include <QtCore>

#include <klocale.h>

#include "solidstats/processes.h"
#include "solidstats/process.h"

class testProcess: public QObject
{
    Q_OBJECT
      private slots:
        void testProcesses();
};

void testProcess::testProcesses() {
	QHash<long, Solid::Process *> processes = Solid::Processes::getProcesses();
	foreach( Solid::Process *process, processes)
		kDebug() << process->name << endl;

	QHash<long, Solid::Process *> processes2 = Solid::Processes::getProcesses();
	QCOMPARE(processes, processes2); //Make sure calling it twice gives the same results.  The difference is time is so small that it really shouldn't have changed
}
