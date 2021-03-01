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

#include "SensorTreeModel.h"

class SensorTreeModelTest : public QObject
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
        KSysGuard::SensorTreeModel model;
        QAbstractItemModelTester tester(&model);
        Q_UNUSED(tester)

        QVERIFY(model.rowCount() == 0);

        QTRY_VERIFY(model.rowCount() > 0);
    }
};

QTEST_MAIN(SensorTreeModelTest);

#include "sensortreemodeltest.moc"
