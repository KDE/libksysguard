/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.9
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.8 as Kirigami

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
        spacing: 0

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

        Item {
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            Layout.minimumHeight: heading.visible ? Kirigami.Units.largeSpacing : 0
        }

        GroupedText {
            totalSensorIds: root.controller.totalSensors
            highPrioritySensorIds: root.controller.highPrioritySensorIds
            lowPrioritySensorIds: root.controller.lowPrioritySensorIds
            showGroups: root.controller.faceConfiguration.groupByTotal
            colorSource: root.colorSource
            totalHeight: root.height
            updateRateLimit: root.controller.updateRateLimit
            sensorLabels: root.controller.sensorLabels
        }

        Item {
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            // Trick ColumnLayout into layouting this spacer and the one above
            // the same height. Apparently if only minimumHeight is set
            // ColumnLayout will still use that as "weight" for sizing.
            Layout.preferredHeight: heading.visible ? Kirigami.Units.largeSpacing : 0
        }
    }
}
