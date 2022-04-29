/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.9
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.13 as Kirigami

import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces

import org.kde.quickcharts 1.0 as Charts
import org.kde.quickcharts.controls 1.0 as ChartsControls

Faces.SensorFace {
    id: root

    readonly property int columnCount: root.controller.faceConfiguration.columnCount

    // When automatically determining the number of columns, use the square root
    // of the number of sensors, rounded up. This should give us a number of
    // columns that generally divides things evenly across the grid.
    readonly property int autoColumnCount: Math.ceil(Math.sqrt(controller.highPrioritySensorIds.length))

    // Arbitrary minimumWidth to make easier to align plasmoids in a predictable way
    Layout.minimumWidth: Kirigami.Units.gridUnit * 8
    Layout.preferredWidth: grid.preferredWidth + Kirigami.Units.largeSpacing

    contentItem: FaceGrid {
        id: grid

        columnCount: root.columnCount
        autoColumnCount: root.autoColumnCount
        useSensorColor: root.controller.faceConfiguration.useSensorColor
    }
}
