/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.2

import org.kde.kirigami 2.4 as Kirigami

import org.kde.ksysguard.formatter 1.0 as Formatter
import org.kde.ksysguard.sensors 1.0 as Sensors

Item {
    id: root

    property alias usedSensor: usedSensorObject.sensorId
    property alias totalSensor: totalSensorObject.sensorId

    property int updateRateLimit

    property real contentMargin: 10

    ColumnLayout {
        id: layout

        anchors.centerIn: parent
        width: Math.min(parent.width, parent.height) * 0.75

        spacing: 0

        Label {
            id: usedLabel

            Layout.fillWidth: true

            text: chart.sensorsModel.sensorLabels[usedSensor] ||  (usedSensorObject.name + (usedSensorObject.shortName.length > 0 ? "\x9C" + usedSensorObject.shortName : ""))
            horizontalAlignment: Text.AlignHCenter
            font: Kirigami.Theme.smallFont
            color: Kirigami.Theme.disabledTextColor
            visible: totalValue.visible
            elide: Text.ElideRight

            opacity: layout.y > root.contentMargin ? 1 : 0
        }

        Label {
            id: usedValue
            Layout.fillWidth: true
            text: usedSensorObject.formattedValue
            horizontalAlignment: Text.AlignHCenter

            fontSizeMode: Text.HorizontalFit
            minimumPointSize: 7
        }

        Kirigami.Separator {
            Layout.alignment: Qt.AlignHCenter;
            Layout.preferredWidth: Math.max(usedValue.contentWidth, totalValue.contentWidth)
            visible: totalValue.visible
        }

        Label {
            id: totalValue

            Layout.fillWidth: true

            text: totalSensorObject.formattedValue
            horizontalAlignment: Text.AlignHCenter
            visible: root.totalSensor.length > 0 && contentWidth < layout.width
        }

        Label {
            id: totalLabel

            Layout.fillWidth: true

            text: chart.sensorsModel.sensorLabels[totalSensor] || (totalSensorObject.name + (totalSensorObject.shortName.length > 0 ? "\x9C" + totalSensorObject.shortName : ""))
            horizontalAlignment: Text.AlignHCenter
            font: Kirigami.Theme.smallFont
            color: Kirigami.Theme.disabledTextColor
            visible: totalValue.visible
            elide: Text.ElideRight

            opacity: layout.y + layout.height < root.height - root.contentMargin ? 1 : 0
        }

        Sensors.Sensor {
            id: usedSensorObject
            updateRateLimit: root.updateRateLimit
        }

        Sensors.Sensor {
            id: totalSensorObject
            updateRateLimit: root.updateRateLimit
        }
    }
}
