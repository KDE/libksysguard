/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces
import org.kde.ksysguard.formatter as Formatter

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls


Faces.CompactSensorFace {
    id: root

    Layout.minimumHeight: verticalFormFactor ? contentItem.implicitHeight : defaultMinimumSize

    Layout.preferredWidth: horizontalFormFactor ? contentItem.preferredWidth : undefined

    contentItem: ChartsControls.LegendLayout {
        horizontalSpacing: 1
        verticalSpacing: 1

        Repeater {
            model: root.controller.highPrioritySensorIds

            ChartsControls.LegendDelegate {
                name: root.controller.sensorLabels[modelData] || sensor.name
                shortName: root.controller.sensorLabels[modelData] || sensor.shortName
                value: sensor.formattedValue
                color: root.colorSource.map[modelData]

                font: Kirigami.Theme.smallFont

                maximumValueWidth: Formatter.Formatter.maximumLength(sensor.unit, Kirigami.Theme.smallFont)

                opacity: y + height > root.height ? 0 : 1

                ChartsControls.LegendLayout.minimumWidth: minimumWidth
                ChartsControls.LegendLayout.preferredWidth: preferredWidth
                ChartsControls.LegendLayout.maximumWidth: Math.max(preferredWidth, Kirigami.Units.iconSizes.huge)

                Sensors.Sensor {
                    id: sensor
                    sensorId: modelData
                    updateRateLimit: root.controller.updateRateLimit
                }
            }
        }
    }
}
