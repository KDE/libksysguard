/*
 *   Copyright 2019 Marco Martin <mart@kde.org>
 *   Copyright 2019 David Edmundson <davidedmundson@kde.org>
 *   Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
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

Charts.LineChart {
    id: chart
    
    property var controller

    readonly property alias sensorsModel: sensorsModel
    readonly property int historyAmount: controller.faceConfiguration.historyAmount

    direction: Charts.XYChart.ZeroAtEnd

    fillOpacity: controller.faceConfiguration.lineChartFillOpacity / 100
    stacked: controller.faceConfiguration.lineChartStacked
    smooth: controller.faceConfiguration.lineChartSmooth

    //TODO: Have a central heading here too?
    //TODO: Have a plasmoid config value for line thickness?

    readonly property bool stackedAuto: chart.controller.faceConfiguration.rangeAutoY && chart.controller.faceConfiguration.lineChartStacked

    yRange {
        from: stackedAuto ? Math.min(sensorsModel.minimum, 0) : chart.controller.faceConfiguration.rangeFromY
        to: stackedAuto ? sensorsModel.stackedMaximum : chart.controller.faceConfiguration.rangeToY
        automatic: (chart.controller.faceConfiguration.rangeAutoY && !chart.controller.faceConfiguration.lineChartStacked)
            || stackedAuto && yRange.from == yRange.to
    }

    Sensors.SensorDataModel {
        id: sensorsModel
        sensors: chart.controller.highPrioritySensorIds
        updateRateLimit: chart.controller.updateRateLimit

        property double stackedMaximum: yRange.stackedAuto ? calcStackedMaximum() : 0

        function calcStackedMaximum() {
            let max = 0
            for (let i = 0; i < sensorsModel.sensors.length; ++i) {
                max += sensorsModel.data(sensorsModel.index(0, i), Sensors.SensorDataModel.Maximum)
            }
            return max
        }
    }

    Connections {
        target: sensorsModel
        enabled: chart.stackedAuto
        function onColumnsInserted() {
            sensorsModel.stackedMaximum = sensorsModel.calcStackedMaximum()
        }
        function onColumnsRemoved() {
            sensorsModel.stackedMaximum = sensorsModel.calcStackedMaximum()
        }
        function onSensorMetaDataChanged() {
            sensorsModel.stackedMaximum = sensorsModel.calcStackedMaximum()
        }
    }

    Instantiator {
        model: sensorsModel.sensors
        delegate: Charts.HistoryProxySource {
            id: history

            source: Charts.ModelSource {
                model: sensorsModel
                column: index
                roleName: "Value"
            }

            interval: {
                if (chart.controller.updateRateLimit > 0) {
                    return chart.controller.updateRateLimit
                }

                if (sensorsModel.ready) {
                    return sensorsModel.headerData(index, Qt.Horizontal, Sensors.SensorDataModel.UpdateInterval)
                }

                return 0
            }
            maximumHistory: interval > 0 ? (chart.historyAmount * 1000) / interval : 0
            fillMode: Charts.HistoryProxySource.FillFromEnd

            property var connection: Connections {
                target: chart.controller
                function onUpdateRateLimitChanged() {
                    history.clear()
                }
            }
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
        roleName: "Name";
        model: sensorsModel
        indexColumns: true
    }
    shortNameSource: Charts.ModelSource {
        roleName: "ShortName";
        model: sensorsModel
        indexColumns: true
    }
}

