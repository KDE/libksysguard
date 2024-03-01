/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls


Faces.SensorFace {
    id: root

    Layout.minimumWidth: root.formFactor == Faces.SensorFace.Vertical ? Kirigami.Units.gridUnit : Kirigami.Units.gridUnit * 2
    Layout.minimumHeight: root.formFactor == Faces.SensorFace.Vertical ? contentItem.implicitHeight : Kirigami.Units.gridUnit

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Item { Layout.fillWidth: true; Layout.preferredHeight: Kirigami.Units.smallSpacing }

        Repeater {
            model: root.controller.highPrioritySensorIds

            Bar {
                Layout.preferredHeight: Kirigami.Units.largeSpacing

                topInset: 0
                bottomInset: 0

                opacity: y + height <= root.height
                sensor: sensor
                controller: root.controller
                color: root.colorSource.map[modelData]

                Sensors.Sensor {
                    id: sensor
                    sensorId: modelData
                    updateRateLimit: root.controller.updateRateLimit
                }
            }
        }

        Item { Layout.fillWidth: true; Layout.preferredHeight: Kirigami.Units.smallSpacing }
    }
}
