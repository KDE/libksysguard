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

// Don't use CompactSensorFace here to ensure we always use the size hints from FaceGrid
Faces.SensorFace {
    id: root

    readonly property int columnCount: root.controller.faceConfiguration.columnCount
    readonly property int autoColumnCount: Math.ceil(Math.sqrt(controller.highPrioritySensorIds.length))

    contentItem: FaceGrid {
        id: grid
        compact: true
        columnCount: root.columnCount
        autoColumnCount: root.autoColumnCount
    }
}
