/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.9
import QtQuick.Layouts 1.4

import org.kde.kirigami 2.8 as Kirigami

import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces

import org.kde.quickcharts 1.0 as Charts
import org.kde.quickcharts.controls 1.0 as ChartsControls

Faces.SensorFace {
    id: root
    readonly property bool showLegend: controller.faceConfiguration.showLegend

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

        PieChart {
            id: compactRepresentation
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: root.formFactor != Faces.SensorFace.Vertical ? Kirigami.Units.gridUnit * 4 : Kirigami.Units.gridUnit
            Layout.minimumHeight: root.formFactor === Faces.SensorFace.Constrained 
                ? Kirigami.Units.gridUnit
                : 5 * Kirigami.Units.gridUnit
            Layout.preferredHeight: 8 * Kirigami.Units.gridUnit
            Layout.maximumHeight: Math.max(root.width, Layout.minimumHeight)
            updateRateLimit: root.controller.updateRateLimit
        }

        Faces.ExtendedLegend {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: root.formFactor === Faces.SensorFace.Horizontal
                || root.formFactor === Faces.SensorFace.Vertical
                ? implicitHeight
                : Kirigami.Units.gridUnit
            visible: root.showLegend
            chart: compactRepresentation.chart
            sourceModel: root.showLegend ? compactRepresentation.sensorsModel : null
            sensorIds: root.showLegend ? root.controller.lowPrioritySensorIds : []
            updateRateLimit: root.controller.updateRateLimit
        }
    }
}
