/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors

Rectangle {
    id: rect

    property Sensors.Sensor sensor
    property alias text: label.text
    property bool useSensorColor

    color: Kirigami.ColorUtils.adjustColor(Kirigami.Theme.textColor, {"alpha": 40})

    Rectangle {
        anchors.fill: parent
        color: rect.useSensorColor ? root.colorSource.map[modelData] : Kirigami.Theme.highlightColor
        opacity: (rect.sensor.value / rect.sensor.maximum)
    }

    Label {
        id: label
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        textFormat: Text.PlainText
    }

}

