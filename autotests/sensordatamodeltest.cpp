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
        QDBusInterface interface{QStringLiteral("org.kde.ksystemstats"), QStringLiteral("/")};
        if (!interface.isValid()) {
            QSKIP("KSystemStats Deamon is not running");
        }
    }

    void testModel()
    {
        KSysGuard::SensorDataModel model;
        QAbstractItemModelTester tester{&model};
        Q_UNUSED(tester)

        QVERIFY(model.columnCount() == 0);

        model.setSensors({
            qs("cpu/all/usage"),
            qs("memory/physical/used"),
            qs("network/all/download"),
            qs("disk/all/used")
        });

        QTRY_VERIFY(model.isReady());

        QCOMPARE(model.columnCount(), 4);

        auto id = KSysGuard::SensorDataModel::SensorId;
        auto unit = KSysGuard::SensorDataModel::Unit;

        QCOMPARE(model.headerData(0, Qt::Horizontal, id).toString(), qs("cpu/all/usage"));
        QCOMPARE(model.headerData(1, Qt::Horizontal, id).toString(), qs("memory/physical/used"));
        QCOMPARE(model.headerData(2, Qt::Horizontal, id).toString(), qs("network/all/download"));
        QCOMPARE(model.headerData(3, Qt::Horizontal, id).toString(), qs("disk/all/used"));

        // Verify that metadata is also loaded correctly. Not using names to sidestep translation issues.
        QCOMPARE(model.headerData(0, Qt::Horizontal, unit).value<KSysGuard::Unit>(), KSysGuard::UnitPercent);
        QCOMPARE(model.headerData(1, Qt::Horizontal, unit).value<KSysGuard::Unit>(), KSysGuard::UnitByte);
        QCOMPARE(model.headerData(2, Qt::Horizontal, unit).value<KSysGuard::Unit>(), KSysGuard::UnitByteRate);
        QCOMPARE(model.headerData(3, Qt::Horizontal, unit).value<KSysGuard::Unit>(), KSysGuard::UnitByte);

        model.setSensors({
            qs("disk/all/used"),
            qs("network/all/download"),
            qs("cpu/all/usage")
        });

        QVERIFY(!model.isReady());

        QTRY_VERIFY(model.isReady());

        QCOMPARE(model.columnCount(), 3);

        QCOMPARE(model.headerData(0, Qt::Horizontal, id).toString(), qs("disk/all/used"));
        QCOMPARE(model.headerData(1, Qt::Horizontal, id).toString(), qs("network/all/download"));
        QCOMPARE(model.headerData(2, Qt::Horizontal, id).toString(), qs("cpu/all/usage"));

        // Verify that metadata is also loaded correctly. Not using names to sidestep translation issues.
        QCOMPARE(model.headerData(0, Qt::Horizontal, unit).value<KSysGuard::Unit>(), KSysGuard::UnitByte);
        QCOMPARE(model.headerData(1, Qt::Horizontal, unit).value<KSysGuard::Unit>(), KSysGuard::UnitByteRate);
        QCOMPARE(model.headerData(2, Qt::Horizontal, unit).value<KSysGuard::Unit>(), KSysGuard::UnitPercent);
    }
};

QTEST_MAIN(SensorDataModelTest);

#include "sensordatamodeltest.moc"
