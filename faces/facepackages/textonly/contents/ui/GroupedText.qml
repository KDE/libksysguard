/*
 *   Copyright 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.8 as Kirigami
import org.kde.quickcharts.controls 1.0 as ChartsControls
import org.kde.ksysguard.sensors 1.0 as Sensors

ColumnLayout {
    id: root

    property var totalSensorIds
    property var highPrioritySensorIds
    property var lowPrioritySensorIds
    property bool showGroups: false
    property var colorSource
    property real totalHeight

    Repeater {
        model: root.showGroups ? root.totalSensorIds : 1

        ColumnLayout {
            property string title
            property var sensors: []
            property bool useFullName: true
            property var colorSource

            Kirigami.Heading {
                Layout.fillWidth: true
                text: groupSensor.formattedValue
                level: 3
                horizontalAlignment: Text.AlignLeft
                opacity: (root.y + parent.y + y + height) < root.totalHeight ? 1 : 0
                visible: text.length > 0
                elide: Text.ElideRight
            }

            Repeater {
                model: {
                    if (!root.showGroups) {
                        return root.highPrioritySensors.concat(root.lowPrioritySensors)
                    }

                    let result = []
                    for (let i = 0; i < root.highPrioritySensors.length; ++i) {
                        if (root.highPrioritySensors[i].name.startsWith(groupSensor.value)) {
                            result.push(root.highPrioritySensors[i])
                        }
                    }
                    for (let i = 0; i < root.lowPrioritySensors.length; ++i) {
                        if (root.lowPrioritySensors[i].name.startsWith(groupSensor.value)) {
                            result.push(root.lowPrioritySensors[i])
                        }
                    }
                    return result
                }

                ChartsControls.LegendDelegate {
                    Layout.fillWidth: true
                    Layout.minimumHeight: implicitHeight

                    name: root.showGroups ? modelData.shortName : modelData.name
                    value: modelData.formattedValue
                    colorVisible: true
                    color: root.colorSource.map[modelData.sensorId]

                    layoutWidth: root.width
                    valueWidth: Kirigami.Units.gridUnit * 2

                    opacity: (root.y + parent.y + y + height) < root.totalHeight ? 1 : 0
                }
            }

            Sensors.Sensor {
                id: groupSensor
                sensorId: root.showGroups ? modelData : ""
            }
        }
    }

    property var highPrioritySensors: []
    property var lowPrioritySensors: []

    Instantiator {
        model: root.highPrioritySensorIds

        Sensors.Sensor { sensorId: modelData }

        onObjectAdded: {
            root.highPrioritySensors.push(object)
            root.highPrioritySensors = root.highPrioritySensors
        }
        onObjectRemoved: {
            root.highPrioritySensors.splice(root.highPrioritySensors.indexOf(object), 1)
            root.highPrioritySensors = root.highPrioritySensors
        }
    }

    Instantiator {
        model: root.lowPrioritySensorIds

        Sensors.Sensor { sensorId: modelData }

        onObjectAdded: {
            root.lowPrioritySensors.push(object)
            root.lowPrioritySensors = root.lowPrioritySensors
        }
        onObjectRemoved: {
            root.lowPrioritySensors.splice(root.lowPrioritySensors.indexOf(object), 1)
            root.lowPrioritySensors = root.lowPrioritySensors
        }
    }
}
