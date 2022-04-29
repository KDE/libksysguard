/*
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.ksysguard.sensors 1.0 as Sensors

import org.kde.quickcharts.controls 1.0 as ChartsControls

GridLayout {
    id: grid

    property int columnCount
    property int autoColumnCount
    property bool useSensorColor

    readonly property real preferredWidth: titleMetrics.width

    columns: columnCount > 0 ? columnCount : autoColumnCount

    columnSpacing: Kirigami.Units.largeSpacing
    rowSpacing: Kirigami.Units.largeSpacing

    Kirigami.Heading {
        id: heading
        Layout.fillWidth: true
        Layout.columnSpan: parent.columns

        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        text: root.controller.title
        visible: root.controller.showTitle && text.length > 0
        level: 2

        TextMetrics {
            id: titleMetrics
            font: heading.font
            text: heading.text
        }
    }

    Repeater {
        model: root.controller.highPrioritySensorIds

        SensorRect {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sensor: sensor
            text: sensor.formattedValue
            useSensorColor: grid.useSensorColor

            Sensors.Sensor {
                id: sensor
                sensorId: modelData
                updateRateLimit: root.controller.updateRateLimit
            }
        }
    }
}
