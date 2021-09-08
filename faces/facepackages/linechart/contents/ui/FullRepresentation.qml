/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.9
import QtQuick.Controls 2.9 as QQC2
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.12 as Kirigami

import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces
import org.kde.ksysguard.formatter 1.0 as Formatter

import org.kde.quickcharts 1.0 as Charts

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
            Layout.minimumHeight: compactRepresentation.Layout.minimumHeight
            Charts.AxisLabels {
                id: axisLabels
                visible: showYAxisLabels
                Layout.fillHeight: true
                constrainToBounds: false
                direction: Charts.AxisLabels.VerticalBottomTop
                delegate: QQC2.Label {
                    anchors.right: parent.right
                    font: Kirigami.Theme.smallFont
                    text: Formatter.Formatter.formatValueShowNull(Charts.AxisLabels.label,
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
                Layout.minimumHeight: root.formFactor === Faces.SensorFace.Constrained
                    ? Kirigami.Units.gridUnit
                    : 3 * Kirigami.Units.gridUnit
                Layout.preferredHeight: 5 * Kirigami.Units.gridUnit

                controller: root.controller

                Charts.GridLines {
                    id: horizontalLines
                    visible: showGridLines
                    direction: Charts.GridLines.Vertical
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
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: root.formFactor === Faces.SensorFace.Horizontal
                || root.formFactor === Faces.SensorFace.Vertical
                ? implicitHeight
                : Kirigami.Units.gridUnit
            visible: root.showLegend
            chart: compactRepresentation
            sourceModel: root.showLegend ? compactRepresentation.sensorsModel : null
            sensorIds: root.showLegend ? root.controller.lowPrioritySensorIds : []
            updateRateLimit: root.controller.updateRateLimit
        }
    }
}
