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
import org.kde.quickcharts.controls 1.0 as ChartControls


ChartControls.PieChartControl {
    id: chart

    property alias headingSensor: sensor.sensorId
    property alias sensors: sensorsModel.sensors
    property alias sensorsModel: sensorsModel

    property int updateRateLimit

    Layout.minimumHeight: root.formFactor == Faces.SensorFace.Vertical ? width : Kirigami.Units.gridUnit

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    readonly property real rangeFrom: root.controller.faceConfiguration.rangeFrom *
                                      root.controller.faceConfiguration.rangeFromMultiplier

    readonly property real rangeTo: root.controller.faceConfiguration.rangeTo *
                                    root.controller.faceConfiguration.rangeToMultiplier

    chart.smoothEnds: root.controller.faceConfiguration.smoothEnds
    chart.fromAngle: root.controller.faceConfiguration.fromAngle
    chart.toAngle: root.controller.faceConfiguration.toAngle

    range {
        from: chart.rangeFrom
        to: chart.rangeTo
        automatic: root.controller.faceConfiguration.rangeAuto
    }

    chart.backgroundColor: Qt.rgba(0.0, 0.0, 0.0, 0.2)

    valueSources: Charts.ModelSource {
        model: Sensors.SensorDataModel {
            id: sensorsModel
            sensors: root.controller.highPrioritySensorIds
            updateRateLimit: chart.updateRateLimit
            sensorLabels: root.controller.sensorLabels
        }
        roleName: "Value"
        indexColumns: true
    }
    chart.nameSource: Charts.ModelSource {
        roleName: "Name";
        model: valueSources[0].model;
        indexColumns: true
    }
    chart.shortNameSource: Charts.ModelSource {
        roleName: "ShortName";
        model: valueSources[0].model;
        indexColumns: true
    }
    chart.colorSource: root.colorSource

    Sensors.Sensor {
        id: sensor
        sensorId: root.controller.totalSensors.length > 0 ? root.controller.totalSensors[0] : ""
        updateRateLimit: chart.updateRateLimit
    }

    UsedTotalDisplay {
        anchors.fill: parent

        usedSensor: root.controller.totalSensors.length > 0 ? root.controller.totalSensors[0] : ""
        totalSensor: root.controller.totalSensors.length > 1 ? root.controller.totalSensors[1] : ""

        contentMargin: chart.chart.thickness
        updateRateLimit: chart.updateRateLimit
    }
}

