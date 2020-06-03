/*
 * Copyright 2020  Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QTest>
#include <QAbstractItemModelTester>

#include <QDBusInterface>

#include "SensorDataModel.h"
#include "Unit.h"

#define qs QStringLiteral

class SensorDataModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QDBusInterface interface{QStringLiteral("org.kde.kstats"), QStringLiteral("/")};
        if (!interface.isValid()) {
            QSKIP("KStats Deamon is not running");
        }
    }

    void testModel()
    {
        auto model = new KSysGuard::SensorDataModel();
        auto tester = new QAbstractItemModelTester(model);

        QVERIFY(model->columnCount() == 0);

        model->setSensors({
            qs("cpu/system/TotalLoad"),
            qs("mem/physical/allocated"),
            qs("network/all/sentDataRate"),
            qs("partitions/all/usedspace")
        });

        QTest::qWait(500); // Allow the model to communicate with the daemon.

        QCOMPARE(model->columnCount(), 4);

        auto id = KSysGuard::SensorDataModel::SensorId;
        auto unit = KSysGuard::SensorDataModel::Unit;

        QCOMPARE(model->headerData(0, Qt::Horizontal, id).toString(), qs("cpu/system/TotalLoad"));
        QCOMPARE(model->headerData(1, Qt::Horizontal, id).toString(), qs("mem/physical/allocated"));
        QCOMPARE(model->headerData(2, Qt::Horizontal, id).toString(), qs("network/all/sentDataRate"));
        QCOMPARE(model->headerData(3, Qt::Horizontal, id).toString(), qs("partitions/all/usedspace"));

        // Verify that metadata is also loaded correctly. Not using names to sidestep translation issues.
        QCOMPARE(model->headerData(0, Qt::Horizontal, unit).toInt(), KSysGuard::UnitPercent);
        QCOMPARE(model->headerData(1, Qt::Horizontal, unit).toInt(), KSysGuard::UnitKiloByte);
        QCOMPARE(model->headerData(2, Qt::Horizontal, unit).toInt(), KSysGuard::UnitKiloByteRate);
        QCOMPARE(model->headerData(3, Qt::Horizontal, unit).toInt(), KSysGuard::UnitKiloByte);

        model->setSensors({
            qs("partitions/all/usedspace"),
            qs("network/all/sentDataRate"),
            qs("cpu/system/TotalLoad")
        });

        QTest::qWait(500); // As above, give some time for communication.

        QCOMPARE(model->columnCount(), 3);

        QCOMPARE(model->headerData(0, Qt::Horizontal, id).toString(), qs("partitions/all/usedspace"));
        QCOMPARE(model->headerData(1, Qt::Horizontal, id).toString(), qs("network/all/sentDataRate"));
        QCOMPARE(model->headerData(2, Qt::Horizontal, id).toString(), qs("cpu/system/TotalLoad"));

        // Verify that metadata is also loaded correctly. Not using names to sidestep translation issues.
        QCOMPARE(model->headerData(0, Qt::Horizontal, unit).toInt(), KSysGuard::UnitKiloByte);
        QCOMPARE(model->headerData(1, Qt::Horizontal, unit).toInt(), KSysGuard::UnitKiloByteRate);
        QCOMPARE(model->headerData(2, Qt::Horizontal, unit).toInt(), KSysGuard::UnitPercent);
    }
};

QTEST_MAIN(SensorDataModelTest);

#include "sensordatamodeltest.moc"
