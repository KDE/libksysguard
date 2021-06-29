/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.9
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.8 as Kirigami

import org.kde.ksysguard.formatter 1.0
import org.kde.ksysguard.sensors 1.0

import org.kde.quickcharts 1.0 as Charts
import org.kde.quickcharts.controls 1.0 as ChartsControls

ChartsControls.Legend {
    id: legend

    property alias textOnlySensorIds: textOnlySensorsRepeater.model
    property var sourceModel
    property var colorSource

    flow: GridLayout.TopToBottom

    Layout.maximumHeight: implicitHeight
    Layout.maximumWidth: parent.width

    spacing: Kirigami.Units.smallSpacing

    valueVisible: true
    valueWidth: units.gridUnit * 2
    formatValue: function(input, index) {
        return Formatter.formatValueShowNull(input, sourceModel.data(sourceModel.index(0, index), SensorDataModel.Unit))
    }

    Repeater {
        id: textOnlySensorsRepeater
        delegate: ChartsControls.LegendDelegate {
            name: sensor.shortName
            value: sensor.formattedValue || ""
            colorVisible: false

            layoutWidth: legend.width
            valueWidth: units.gridUnit * 2

            Sensor {
                id: sensor
                sensorId: modelData
            }
        }
    }
}
