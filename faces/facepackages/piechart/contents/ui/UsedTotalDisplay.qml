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

        anchors.fill: parent

        spacing: 0

        Item { Layout.fillWidth: true; Layout.fillHeight: true }

        Label {
            id: usedLabel

            Layout.fillWidth: true
            Layout.leftMargin: parent.width * 0.25
            Layout.rightMargin: parent.width * 0.25

            text: chart.sensorsModel.sensorLabels[usedSensor] ||  (usedSensorObject.name + (usedSensorObject.shortName.length > 0 ? "\x9C" + usedSensorObject.shortName : ""))
            horizontalAlignment: Text.AlignHCenter
            font: Kirigami.Theme.smallFont
            color: Kirigami.Theme.disabledTextColor
            visible: totalValue.visible
            elide: Text.ElideRight

            opacity: y > root.contentMargin ? 1 : 0
        }

        Label {
            id: usedValue

            Layout.fillWidth: true
            Layout.leftMargin: root.contentMargin
            Layout.rightMargin: root.contentMargin

            text: usedSensorObject.formattedValue
            horizontalAlignment: Text.AlignHCenter
            font: layout.width >= normalFontWidth ? Kirigami.Theme.defaultFont : Kirigami.Theme.smallFont
            visible: layout.width >= smallFontWidth

            elide: Text.ElideRight

            property real normalFontWidth: {
                var length = Formatter.Formatter.maximumLength(usedSensorObject.unit, Kirigami.Theme.defaultFont)
                if (length > 0) {
                    return length
                } else {
                    return implicitWidth
                }
            }
            property real smallFontWidth: Formatter.Formatter.maximumLength(usedSensorObject.unit, Kirigami.Theme.smallFont)
        }

        Kirigami.Separator {
            Layout.alignment: Qt.AlignHCenter;
            Layout.preferredWidth: Math.max(usedValue.contentWidth, totalValue.contentWidth)
            visible: totalValue.visible
        }

        Label {
            id: totalValue

            Layout.fillWidth: true
            Layout.leftMargin: root.contentMargin
            Layout.rightMargin: root.contentMargin

            text: totalSensorObject.formattedValue
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight

            visible: root.totalSensor.length > 0 && layout.width >= usedValue.normalFontWidth
        }

        Label {
            id: totalLabel

            Layout.fillWidth: true
            Layout.leftMargin: parent.width * 0.25
            Layout.rightMargin: parent.width * 0.25

            text: chart.sensorsModel.sensorLabels[totalSensor] || (totalSensorObject.name + (totalSensorObject.shortName.length > 0 ? "\x9C" + totalSensorObject.shortName : ""))
            horizontalAlignment: Text.AlignHCenter
            font: Kirigami.Theme.smallFont
            color: Kirigami.Theme.disabledTextColor
            visible: totalValue.visible
            elide: Text.ElideRight

            opacity: y + height < root.height - root.contentMargin ? 1 : 0
        }

        Item { Layout.fillWidth: true; Layout.fillHeight: true }

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
