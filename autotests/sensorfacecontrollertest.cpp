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
#include <Solid/Block>
#include <Solid/Device>
#include <Solid/Predicate>
#include <Solid/StorageAccess>
#include <Solid/StorageVolume>

#include "SensorFaceController.h"

#include "SensorFaceController_p.h"

class SensorFaceControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSensorIdConversion_data()
    {
        QTest::addColumn<QJsonArray>("oldSensors");
        QTest::addColumn<QJsonArray>("expectedSensors");

        QTest::addRow("network")
        << QJsonArray {
            QStringLiteral("network/all/sentDataRate"),
            QStringLiteral("network/all/totalReceivedData"),
            QStringLiteral("network/interfaces/test/receiver/data"),
            QStringLiteral("network/interfaces/test/transmitter/dataTotal"),
        }
        << QJsonArray {
            QStringLiteral("network/all/upload"),
            QStringLiteral("network/all/totalDownload"),
            QStringLiteral("network/test/download"),
            QStringLiteral("network/test/totalUpload"),
        };

        const auto storageAccesses = Solid::Device::listFromQuery(Solid::Predicate(Solid::DeviceInterface::StorageAccess, QStringLiteral("accessible"), true));
        for (int i = 0; i < storageAccesses.size(); ++i) {
            const auto storageAccess = storageAccesses[i].as<Solid::StorageAccess>();
            const auto blockDevice = storageAccesses[i].as<Solid::Block>();
            const auto storageVolume = storageAccesses[i].as<Solid::StorageVolume>();
            const QString newPrefix = QStringLiteral("disk/") + (storageVolume->uuid().isEmpty() ? storageVolume->label() : storageVolume->uuid());
            // Old code uses "disk/sdc2_(8:34)/..."
            QString device = blockDevice->device().mid(strlen("/dev/"));
            const QString diskPrefix = QStringLiteral("disk/%1_(%2:%3)").arg(device).arg(blockDevice->deviceMajor()).arg(blockDevice->deviceMinor());
            QTest::addRow("disk%d",i)
            << QJsonArray {
                {diskPrefix + QStringLiteral("/Rate/rio")},
                {diskPrefix + QStringLiteral("/Rate/wio")},
            }
            << QJsonArray {
                {newPrefix + QStringLiteral("/read")},
                {newPrefix + QStringLiteral("/write")},
            };
            // Old code uses "partitions/mountPath/..."
            const QString mountPath = storageAccess->filePath() == QLatin1String("/") ? QStringLiteral("/__root__") : storageAccess->filePath();
            QString partitionPrefix = QStringLiteral("partitions") + mountPath;
            QTest::addRow("partition%d", i)
            << QJsonArray {
                {partitionPrefix + QStringLiteral("/total")},
                {partitionPrefix + QStringLiteral("/freespace")},
                {partitionPrefix + QStringLiteral("/filllevel")},
                {partitionPrefix + QStringLiteral("/usedspace")},
            } << QJsonArray {
                {newPrefix + QStringLiteral("/total")},
                {newPrefix + QStringLiteral("/free")},
                {newPrefix + QStringLiteral("/usedPercent")},
                {newPrefix + QStringLiteral("/used")},
            };
        }
    }
    void testSensorIdConversion()
    {
        QFETCH(QJsonArray, oldSensors);
        QFETCH(QJsonArray, expectedSensors);
        KConfig config;
        auto sensorsGroup = config.group("Sensors");
        sensorsGroup.writeEntry("sensors", QJsonDocument{oldSensors}.toJson(QJsonDocument::Compact));

        KSysGuard::SensorFaceControllerPrivate d;

        auto sensors = d.readAndUpdateSensors(sensorsGroup, QStringLiteral("sensors"));

        QCOMPARE(sensors.size(), expectedSensors.size());

        for (int i = 0; i < sensors.size(); ++i) {
            QCOMPARE(sensors.at(i), expectedSensors.at(i));
        }

        auto newEntry = sensorsGroup.readEntry("sensors");
        QCOMPARE(newEntry.toUtf8(), QJsonDocument{expectedSensors}.toJson(QJsonDocument::Compact));
    }
};

QTEST_MAIN(SensorFaceControllerTest);

#include "sensorfacecontrollertest.moc"
