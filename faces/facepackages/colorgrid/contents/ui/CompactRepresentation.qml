/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

Faces.CompactSensorFace {
    id: root

    Layout.minimumWidth: Math.max(grid.columns * defaultMinimumSize + (grid.columns - 1) * grid.columnSpacing, defaultMinimumSize)
    Layout.minimumHeight: Math.max(grid.rowCount * defaultMinimumSize  + (grid.rowCount - 1) * grid.rowSpacing, defaultMinimumSize)

    readonly property int columnCount: root.controller.faceConfiguration.columnCount
    readonly property int autoColumnCount: Math.ceil(Math.sqrt(controller.highPrioritySensorIds.length))

    contentItem: FaceGrid {
        id: grid

        controller: root.controller
        colorSource: root.colorSource

        compact: true

        columns: {
            let itemCount = root.controller.highPrioritySensorIds.length

            let maxColumns = Math.floor(width / defaultMinimumSize)
            let maxRows = Math.floor(height / defaultMinimumSize)

            let columns = root.columnCount > 0 ? root.columnCount : root.autoColumnCount
            let rows = Math.ceil(itemCount / columns)

            if (horizontalFormFactor) {
                rows = Math.min(rows, maxRows)
                columns = Math.ceil(itemCount / rows)
            } else if (verticalFormFactor) {
                columns = Math.min(columns, maxColumns)
            }

            return columns
        }
    }
}
