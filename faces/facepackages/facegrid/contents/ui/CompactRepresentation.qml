/*
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces
import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartControls

Faces.SensorFace {
    id: root

    readonly property int columnCount: root.controller.faceConfiguration.columnCount
    readonly property int autoColumnCount: Math.ceil(Math.sqrt(controller.highPrioritySensorIds.length))

    //Layout.minimumWidth: Kirigami.Units.gridUnit * 8
    //Layout.preferredWidth: titleMetrics.width + Kirigami.Units.largeSpacing

    contentItem: FaceGrid {
        id: grid

        columnCount: root.columnCount
        autoColumnCount: root.autoColumnCount
    }
}
