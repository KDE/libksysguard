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

#include <QJsonDocument>
#include <QJsonArray>
#include <KConfig>

#include "SensorFaceController.h"

#include "SensorFaceController_p.h"

class SensorFaceControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSensorIdConversion()
    {
        QJsonArray originalTotalSensors = {
            QStringLiteral("network/all/sentDataRate"),
            QStringLiteral("network/all/totalReceivedData"),
            QStringLiteral("network/interfaces/test/receiver/data"),
            QStringLiteral("network/interfaces/test/transmitter/dataTotal"),
        };
        QJsonArray expectedTotalSensors = {
            QStringLiteral("network/all/upload"),
            QStringLiteral("network/all/totalDownload"),
            QStringLiteral("network/test/download"),
            QStringLiteral("network/test/totalUpload"),
        };

        KConfig config;
        auto sensorsGroup = config.group("Sensors");
        sensorsGroup.writeEntry("totalSensors", QJsonDocument{originalTotalSensors}.toJson(QJsonDocument::Compact));

        KSysGuard::SensorFaceControllerPrivate d;

        auto sensors = d.readAndUpdateSensors(sensorsGroup, QStringLiteral("totalSensors"));

        QCOMPARE(sensors.size(), 4);
        QCOMPARE(sensors.at(0), expectedTotalSensors.at(0));
        QCOMPARE(sensors.at(1), expectedTotalSensors.at(1));
        QCOMPARE(sensors.at(2), expectedTotalSensors.at(2));
        QCOMPARE(sensors.at(3), expectedTotalSensors.at(3));

        auto newEntry = sensorsGroup.readEntry("totalSensors");
        QCOMPARE(newEntry.toUtf8(), QJsonDocument{expectedTotalSensors}.toJson(QJsonDocument::Compact));
    }
};

QTEST_MAIN(SensorFaceControllerTest);

#include "sensorfacecontrollertest.moc"
