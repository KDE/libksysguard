/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls

ProgressBar {
    id: bar

    Layout.fillWidth: true

    property Sensors.Sensor sensor
    property Faces.SensorFaceController controller
    property color color

    readonly property real rangeFrom: controller.faceConfiguration.rangeFrom * controller.faceConfiguration.rangeFromMultiplier
    readonly property real rangeTo: controller.faceConfiguration.rangeTo * controller.faceConfiguration.rangeToMultiplier

    Kirigami.Theme.inherit: false

    value: sensor?.value ?? 0
    from: controller.faceConfiguration.rangeAuto ? sensor.minimum : rangeFrom
    to: controller.faceConfiguration.rangeAuto ? sensor.maximum : rangeTo

    topPadding: topInset
    bottomPadding: bottomInset

    contentItem: Item {
        Rectangle {
            width: bar.visualPosition * parent.width
            height: parent.height
            color: bar.color
            radius: height / 2
        }
    }
}
