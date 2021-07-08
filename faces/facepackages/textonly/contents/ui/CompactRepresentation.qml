/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.9
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.8 as Kirigami

import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces

import org.kde.quickcharts 1.0 as Charts
import org.kde.quickcharts.controls 1.0 as ChartsControls


Faces.SensorFace {
    id: root

    Layout.minimumWidth: root.formFactor == Faces.SensorFace.Vertical ? Kirigami.Units.gridUnit : contentItem.contentWidth
    Layout.minimumHeight: Kirigami.Units.gridUnit

    contentItem: GroupedText {
        totalSensorIds: root.controller.totalSensors
        highPrioritySensorIds: root.controller.highPrioritySensorIds
        lowPrioritySensorIds: []
        showGroups: root.controller.faceConfiguration.groupByTotal
        colorSource: root.colorSource
        totalHeight: root.height
        updateRateLimit: root.controller.updateRateLimit
        sensorLabels: root.controller.sensorLabels
    }
}
