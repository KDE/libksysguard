/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.9
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.13 as Kirigami

import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces

import org.kde.quickcharts 1.0 as Charts
import org.kde.quickcharts.controls 1.0 as ChartsControls

Faces.SensorFace {
    id: root

    // Arbitrary minimumWidth to make easier to align plasmoids in a predictable way
    Layout.minimumWidth: Kirigami.Units.gridUnit * 8
    Layout.preferredWidth: titleMetrics.width

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallspacing

        Kirigami.Heading {
            id: heading
            Layout.fillWidth: true
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

        Item { Layout.fillWidth: true; Layout.fillHeight: true }

        Repeater {
            model: root.controller.highPrioritySensorIds

            ColumnLayout {
                Layout.fillWidth: true
                Layout.bottomMargin: Kirigami.Units.smallSpacing
                spacing: 0
                Bar {
                    sensor: sensor
                }
                ChartsControls.LegendDelegate {
                    readonly property bool isTextOnly: index >= root.controller.highPrioritySensorIds.length

                    Layout.fillWidth: true
                    Layout.minimumHeight: isTextOnly ? 0 : implicitHeight

                    name: root.controller.sensorLabels[sensor.sensorId] || sensor.name
                    shortName: root.controller.sensorLabels[sensor.sensorId] || sensor.shortName
                    value: sensor.formattedValue
                    colorVisible: false

                    layoutWidth: root.width
                    valueWidth: Kirigami.Units.gridUnit * 2

                    Sensors.Sensor {
                        id: sensor
                        sensorId: modelData
                        updateRateLimit: root.controller.updateRateLimit
                    }
                }
            }
        }
        Kirigami.Separator {
            Layout.fillWidth: true
            visible: root.controller.lowPrioritySensorIds.length > 0
        }
        Repeater {
            model: root.controller.lowPrioritySensorIds
            ChartsControls.LegendDelegate {

                Layout.fillWidth: true
                Layout.minimumHeight: implicitHeight

                name: root.controller.sensorLabels[sensor.sensorId] || sensor.shortName
                value: sensor.formattedValue
                colorVisible: false

                layoutWidth: root.width
                valueWidth: Kirigami.Units.gridUnit * 2

                Sensors.Sensor {
                    id: sensor
                    sensorId: modelData
                    updateRateLimit: root.controller.updateRateLimit
                }
            }
        }


        Item { Layout.fillWidth: true; Layout.fillHeight: true }
    }
}
