/*
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces
import org.kde.quickcharts 1.0 as Charts
import org.kde.quickcharts.controls 1.0 as ChartControls

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
