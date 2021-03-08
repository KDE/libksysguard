/*
 *   Copyright 2019 Marco Martin <mart@kde.org>
 *   Copyright 2019 David Edmundson <davidedmundson@kde.org>
 *   Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *   Copyrigth 2019 Kai Uwe Broulik <kde@broulik.de>
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

Charts.BarChart {
    id: chart

    readonly property int barCount: stacked ? 1 : instantiator.count

    readonly property alias sensorsModel: sensorsModel

    property int updateRateLimit

    stacked: root.controller.faceConfiguration.barChartStacked

    spacing: Math.round(width / 20)

    yRange {
        from: root.controller.faceConfiguration.rangeFrom
        to: root.controller.faceConfiguration.rangeTo
        automatic: root.controller.faceConfiguration.rangeAuto
    }

    Sensors.SensorDataModel {
        id: sensorsModel
        sensors: root.controller.highPrioritySensorIds
        updateRateLimit: chart.updateRateLimit
    }

    Instantiator {
        id: instantiator
        model: sensorsModel.sensors
        delegate: Charts.ModelSource {
            model: sensorsModel
            roleName: "Value"
            column: index
        }
        onObjectAdded: {
            chart.insertValueSource(index, object)
        }
        onObjectRemoved: {
            chart.removeValueSource(object)
        }
    }

    colorSource: root.colorSource
    nameSource: Charts.ModelSource {
        model: sensorsModel
        roleName: "Name"
        indexColumns: true
    }
    shortNameSource: Charts.ModelSource {
        roleName: "ShortName";
        model: sensorsModel
        indexColumns: true
    }
}

