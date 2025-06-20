/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Qt5Compat.GraphicalEffects

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.formatter as Formatter
import org.kde.ksysguard.sensors as Sensors

Item {
    id: root

    property alias usedSensor: usedSensorObject.sensorId
    property alias totalSensor: totalSensorObject.sensorId

    property int updateRateLimit

    property real contentMargin: 10

    readonly property bool constrained: width < Kirigami.Units.gridUnit * 2

    FontMetrics {
        id: metrics
        font: Kirigami.Theme.defaultFont
    }

    ColumnLayout {
        id: layout

        anchors.fill: parent
        anchors.margins: root.contentMargin

        spacing: 0

        Item { Layout.fillWidth: true; Layout.fillHeight: true }

        Label {
            id: usedLabel

            Layout.fillWidth: true
            Layout.leftMargin: root.contentMargin
            Layout.rightMargin: root.contentMargin

            text: chart.sensorsModel.sensorLabels[usedSensor] ||  (usedSensorObject.name + (usedSensorObject.shortName.length > 0 ? "\x9C" + usedSensorObject.shortName : ""))
            horizontalAlignment: Text.AlignHCenter
            font: Kirigami.Theme.smallFont
            opacity: 0.75
            visible: totalValue.visible && root.height > metrics.lineSpacing * 5
            elide: Text.ElideRight
        }

        Label {
            id: usedValue
            Layout.fillWidth: true
            Layout.maximumHeight: root.height - root.contentMargin * 2
            Layout.leftMargin: root.constrained ? -root.contentMargin : 0
            Layout.rightMargin: root.constrained ? -root.contentMargin : 0

            text: {
                if (!root.usedSensor) {
                    return ""
                }

                // If size gets too small we really can't do much more than just
                // hide things.
                if (root.height < metrics.lineSpacing) {
                    return ""
                }

                // If we're small but not too small, reduce precision to reduce
                // the amount of characters we need to fit.
                if (root.constrained) {
                    return Formatter.Formatter.formatValueWithPrecision(usedSensorObject.value, usedSensorObject.unit, 0)
                }

                return usedSensorObject.formattedValue
            }
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            fontSizeMode: Text.HorizontalFit
            minimumPointSize: Kirigami.Theme.smallFont.pointSize * 0.8

            // This slightly odd combination ensures that when the width becomes
            // too small, the unit gets hidden because the text wraps but the
            // wrapped part is hidden due to maximumLineCount.
            wrapMode: Text.Wrap
            maximumLineCount: 1

            // When we're small we want to overlap the text onto the chart. To
            // ensure the text remains readable, we want to have some sort of
            // outline behind the text. Unfortunately, there is no way to
            // achieve the visual effect we want here without using deprecated
            // GraphicalEffects. MultiEffect is completely unusable and using
            // `style: Text.Outline` makes the font rendering look pretty bad.
            layer.enabled: root.constrained
            layer.effect: Glow {
                radius: 4
                spread: 0.75
                color: Kirigami.Theme.backgroundColor
            }
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
            visible: root.totalSensor.length > 0 && root.height > metrics.lineSpacing * 4
        }

        Label {
            id: totalLabel

            Layout.fillWidth: true
            Layout.leftMargin: root.contentMargin
            Layout.rightMargin: root.contentMargin

            text: chart.sensorsModel.sensorLabels[totalSensor] || (totalSensorObject.name + (totalSensorObject.shortName.length > 0 ? "\x9C" + totalSensorObject.shortName : ""))
            horizontalAlignment: Text.AlignHCenter
            font: Kirigami.Theme.smallFont
            opacity: 0.75
            visible: totalValue.visible && root.height > metrics.lineSpacing * 5
            elide: Text.ElideRight
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
