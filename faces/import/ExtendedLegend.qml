/*
 *   Copyright 2019 Marco Martin <mart@kde.org>
 *   Copyright 2019 David Edmundson <davidedmundson@kde.org>
 *   Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *   Copyright 2019 Kai Uwe Broulik <kde@broulik.de>
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

import org.kde.ksysguard.formatter 1.0
import org.kde.ksysguard.sensors 1.0

import org.kde.quickcharts 1.0 as Charts
import org.kde.quickcharts.controls 1.0 as ChartsControls

ChartsControls.Legend {
    id: legend

    property alias sensorIds: sensorsRepeater.model
    property var sourceModel
    property var colorSource

    flow: GridLayout.TopToBottom

    Layout.maximumHeight: implicitHeight
    Layout.maximumWidth: parent.width

    spacing: Kirigami.Units.smallSpacing

    valueVisible: true
    valueWidth: Kirigami.Units.gridUnit * 2
    formatValue: function(input, index) {
        if (!sourceModel) {
            return input
        }

        return Formatter.formatValueShowNull(input, sourceModel.data(sourceModel.index(0, index), SensorDataModel.Unit))
    }

    Repeater {
        id: sensorsRepeater
        delegate: ChartsControls.LegendDelegate {
            name: sensor.name
            shortName: sensor.shortName
            value: sensor.formattedValue || ""
            colorVisible: false

            layoutWidth: legend.width
            valueWidth: Kirigami.Units.gridUnit * 2

            Sensor {
                id: sensor
                sensorId: modelData
            }
        }
    }
}
