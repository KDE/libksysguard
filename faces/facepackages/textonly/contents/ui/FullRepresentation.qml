/*
 *   Copyright 2019 Marco Martin <mart@kde.org>
 *   Copyright 2019 David Edmundson <davidedmundson@kde.org>
 *   Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
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

        Item { Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: Kirigami.Units.largeSpacing }

        GroupedText {
            totalSensorIds: root.controller.totalSensors
            highPrioritySensorIds: root.controller.highPrioritySensorIds
            lowPrioritySensorIds: root.controller.lowPrioritySensorIds
            showGroups: root.controller.faceConfiguration.groupByTotal
            colorSource: root.colorSource
            totalHeight: root.height
            updateRateLimit: root.controller.updateRateLimit
        }

        Item {
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            // Trick ColumnLayout into layouting this spacer and the one above
            // the same height. Apparently if only minimumHeight is set
            // ColumnLayout will still use that as "weight" for sizing.
            Layout.preferredHeight: Kirigami.Units.largeSpacing
        }
    }
}
