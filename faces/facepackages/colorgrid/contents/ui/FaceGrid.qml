/*
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces
import org.kde.ksysguard.formatter as Formatter

GridLayout {
    id: grid

    property QtObject controller
    property QtObject colorSource
    property bool compact

    readonly property real preferredWidth: titleMetrics.width

    readonly property int rowCount: Math.ceil(gridRepeater.count / columns)

    columnSpacing: compact ? 1 : Kirigami.Units.largeSpacing
    rowSpacing: compact ? 1 : Kirigami.Units.largeSpacing

    Kirigami.Heading {
        id: heading
        Layout.fillWidth: true
        Layout.columnSpan: parent.columns > 0 ? parent.columns : 1

        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        text: grid.controller.title
        visible: !grid.compact && grid.controller.showTitle && text.length > 0
        level: 2

        TextMetrics {
            id: titleMetrics
            font: heading.font
            text: heading.text
        }
    }

    Repeater {
        id: gridRepeater

        model: grid.controller.highPrioritySensorIds

        SensorRect {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 0
            Layout.preferredHeight: 0

            sensor: Sensors.Sensor {
                sensorId: modelData
                updateRateLimit: grid.controller.updateRateLimit
            }

            text: sensor.formattedValue
            sensorColor: grid.useSensorColor ? grid.colorSource.map[modelData] : Kirigami.Theme.highlightColor
        }
    }
}
