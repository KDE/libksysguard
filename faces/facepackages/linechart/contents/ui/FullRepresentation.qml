/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces
import org.kde.ksysguard.formatter as Formatter

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls

Faces.SensorFace {
    id: root
    readonly property bool showLegend: controller.faceConfiguration.showLegend
    readonly property bool showGridLines: root.controller.faceConfiguration.showGridLines
    readonly property bool showYAxisLabels: root.controller.faceConfiguration.showYAxisLabels
    // Arbitrary minimumWidth to make easier to align plasmoids in a predictable way
    Layout.minimumWidth: Kirigami.Units.gridUnit * 8
    Layout.preferredWidth: titleMetrics.width

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

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

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.fillHeight: true
            Layout.topMargin: showYAxisLabels ? axisMetrics.height / 2 : 0
            Layout.bottomMargin: Layout.topMargin

            Layout.minimumHeight: 3 * Kirigami.Units.gridUnit
            Layout.preferredHeight: 5 * Kirigami.Units.gridUnit

            ChartsControls.AxisLabels {
                id: axisLabels
                visible: showYAxisLabels
                Layout.fillHeight: true
                constrainToBounds: false
                direction: ChartsControls.AxisLabels.VerticalBottomTop
                delegate: QQC2.Label {
                    anchors.right: parent.right
                    font: Kirigami.Theme.smallFont
                    text: Formatter.Formatter.formatValueShowNull(ChartsControls.AxisLabels.label,
                        compactRepresentation.sensorsModel.unit)
                    color: Kirigami.Theme.disabledTextColor
                }
                source: Charts.ChartAxisSource {
                    chart: compactRepresentation
                    axis: Charts.ChartAxisSource.YAxis
                    itemCount: 5
                }
                TextMetrics {
                    id: axisMetrics
                    font: Kirigami.Theme.smallFont
                    text: Formatter.Formatter.formatValueShowNull("0",
                        compactRepresentation.sensorsModel.unit)
                }
            }
            LineChart {
                id: compactRepresentation
                Layout.fillWidth: true
                Layout.fillHeight: true

                controller: root.controller

                highlight: legend.highlightedIndex

                ChartsControls.GridLines {
                    id: horizontalLines
                    visible: showGridLines
                    direction: ChartsControls.GridLines.Vertical
                    anchors.fill: compactRepresentation
                    z: compactRepresentation.z - 1
                    chart: compactRepresentation

                    major.count: 3
                    major.lineWidth: 1
                    // The same color as a Kirigami.Separator
                    major.color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.2)
                    minor.visible: false

                }
            }
        }

        Faces.ExtendedLegend {
            id: legend
            Layout.fillWidth: true
            Layout.minimumHeight: implicitHeight
            visible: root.showLegend
            chart: compactRepresentation
            sourceModel: root.showLegend ? compactRepresentation.sensorsModel : null
            sensorIds: root.showLegend ? root.controller.lowPrioritySensorIds : []
            updateRateLimit: root.controller.updateRateLimit
        }
    }
}
