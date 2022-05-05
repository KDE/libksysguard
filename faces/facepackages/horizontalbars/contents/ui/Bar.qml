/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.9
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.13 as Kirigami

import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces

import org.kde.quickcharts 1.0 as Charts
import org.kde.quickcharts.controls 1.0 as ChartsControls

Rectangle {
    id: bar
    Layout.fillWidth: true
    implicitHeight: Math.round(Kirigami.Units.gridUnit / 3)
    color: Kirigami.ColorUtils.adjustColor(Kirigami.Theme.textColor, {"alpha": 40})
    radius: height/2
    property Sensors.Sensor sensor
    property int value: Math.min(Math.max(Math.round(width * (sensor.value / sensor.maximum)), 0), width)

    Rectangle {
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }
        color: root.colorSource.map[modelData]
        radius: parent.radius
        /* Ensures that the bar has even spacing on top and bottom. */
        height: {
            let value = Math.min(parent.height, parent.value)
            return value + (parent.height - value) % 2;
        }
        width: Math.max(height, parent.value)
    }
}

