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

import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces

import org.kde.quickcharts 1.0 as Charts

Charts.BarChart {
    id: chart

    readonly property int barCount: stacked ? 1 : instantiator.count

    readonly property alias sensorsModel: sensorsModel

    property int updateRateLimit

    property var controller

    stacked: controller.faceConfiguration.barChartStacked

    spacing: Math.round(width / 20)

    yRange {
        from: controller.faceConfiguration.rangeFrom
        to: controller.faceConfiguration.rangeTo
        automatic: controller.faceConfiguration.rangeAuto
    }

    Sensors.SensorDataModel {
        id: sensorsModel
        sensors: controller.highPrioritySensorIds
        updateRateLimit: chart.updateRateLimit
        sensorLabels: root.controller.sensorLabels
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

