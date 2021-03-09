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
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.2 as QQC2
import QtQml.Models 2.12

import Qt.labs.platform 1.1 as Platform

import org.kde.kirigami 2.8 as Kirigami
import org.kde.kquickcontrols 2.0

import org.kde.kitemmodels 1.0 as KItemModels

import org.kde.quickcharts 1.0 as Charts
import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces

import "./" as Local

ColumnLayout {
    id: root

    signal configurationChanged

    property var cfg_totalSensors: []
    property var cfg_highPrioritySensorIds: []
    property var cfg_sensorColors: new Object()
    property var cfg_lowPrioritySensorIds: []

    onCfg_totalSensorsChanged: configurationChanged();
    onCfg_highPrioritySensorIdsChanged: configurationChanged();
    onCfg_sensorColorsChanged: configurationChanged();
    onCfg_lowPrioritySensorIdsChanged: configurationChanged();

    property Faces.SensorFaceController controller

    // In QML someArray = someOtherArray will always trigger a changed signal
    // even if the two arrays are the same
    // to avoid that we implement an explicit check
    function arrayCompare(array1, array2) {
      if (array1.length !== array2.length) {
          return false;
      }
      return array1.every(function(value, index) { return value === array2[index]});
  }

  function saveConfig() {
        controller.totalSensors = cfg_totalSensors;
        controller.highPrioritySensorIds = cfg_highPrioritySensorIds;
        controller.sensorColors = cfg_sensorColors;
        controller.lowPrioritySensorIds = cfg_lowPrioritySensorIds;
    }

    function loadConfig() {
        if (!arrayCompare(cfg_totalSensors, controller.totalSensors)) {
            cfg_totalSensors = controller.totalSensors;
            totalChoice.selected = controller.totalSensors;
        }

        if (!arrayCompare(cfg_highPrioritySensorIds, controller.highPrioritySensorIds)) {
            cfg_highPrioritySensorIds = controller.highPrioritySensorIds;
            highPriorityChoice.selected = controller.highPrioritySensorIds;
        }

        if(JSON.stringify(cfg_sensorColors) != JSON.stringify(controller.sensorColors)) {
            cfg_sensorColors = controller.sensorColors;
        }

        if (!arrayCompare(cfg_lowPrioritySensorIds, controller.lowPrioritySensorIds)) {
            cfg_lowPrioritySensorIds = controller.lowPrioritySensorIds;
            lowPriorityChoice.selected = controller.lowPrioritySensorIds;
        }
    }

    // When the ui is open in systemsettings and the page is switched around,
    // it gets reparented to null. use this to reload its config every time the
    // page is current again. So any non saved change to the sensor list gets forgotten.
    onParentChanged: {
        if (parent) {
            loadConfig()
        }
    }

    Component.onCompleted: loadConfig()

    Connections {
        target: controller
        function onTotalSensorsChanged() {
            Qt.callLater(root.loadConfig);
        }
        function onHighPrioritySensorIdsChanged() {
            Qt.callLater(root.loadConfig);
        }
        function onSensorColorsChanged() {
            Qt.callLater(root.loadConfig);
        }
        function onLowPrioritySensorIdsChanged() {
            Qt.callLater(root.loadConfig);
        }
    }

    Platform.ColorDialog {
        id: colorDialog
        property string destinationSensor

        currentColor: destinationSensor != "" ? controller.sensorColors[destinationSensor] : ""
        onAccepted: {
            cfg_sensorColors[destinationSensor] = color;
            root.cfg_sensorColorsChanged();
        }
    }


    QQC2.Label {
        text: i18ndp("KSysGuardSensorFaces", "Total Sensor", "Total Sensors", controller.maxTotalSensors)
        visible: controller.supportsTotalSensors
    }
    Local.Choices {
        id: totalChoice
        Layout.fillWidth: true
        visible: controller.supportsTotalSensors
        supportsColors: false
        maxAllowedSensors: controller.maxTotalSensors

        onSelectedChanged: root.cfg_totalSensors = selected
    }

    QQC2.Label {
        text: i18nd("KSysGuardSensorFaces", "Sensors")
    }
    Local.Choices {
        id: highPriorityChoice
        Layout.fillWidth: true
        supportsColors: controller.supportsSensorsColors

        onSelectedChanged: root.cfg_highPrioritySensorIds = selected

        colors: root.cfg_sensorColors
        onSelectColor: {
            colorDialog.destinationSensor = sensorId
            colorDialog.open()
        }
        onColorForSensorGenerated: {
            cfg_sensorColors[sensorId] = color
            root.cfg_sensorColorsChanged();
        }
    }

    QQC2.Label {
        text: i18nd("KSysGuardSensorFaces", "Text-Only Sensors")
        visible: controller.supportsLowPrioritySensors
    }
    Local.Choices {
        id: lowPriorityChoice
        Layout.fillWidth: true
        visible: controller.supportsLowPrioritySensors
        supportsColors: false

        onSelectedChanged: root.cfg_lowPrioritySensorIds = selected
    }

    Item {
        Layout.fillHeight: true
    }
}
