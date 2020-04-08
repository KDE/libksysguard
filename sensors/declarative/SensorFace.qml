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

import org.kde.quickcharts 1.0 as Charts
import org.kde.ksysguard.sensors 1.0 as Sensors

Sensors.AbstractSensorFace {
    id: root
    property alias colorSource: colorSource

    Charts.ColorGradientSource {
        id: colorSource
        // originally ColorGenerator used Kirigami.Theme.highlightColor
        baseColor: theme.highlightColor
        itemCount: root.controller.sensorIds.length

        onItemCountChanged: generate()
        Component.onCompleted: generate()

        function generate() {
            var colors = colorSource.colors;
            var savedColors = root.controller.sensorColors;
            for (var i = 0; i < root.controller.sensorIds.length; ++i) {
                if (savedColors.length <= i) {
                    savedColors.push(colors[i]);
                } else {
                    // Use the darker trick to make Qt validate the scring as a valid color;
                    var currentColor = Qt.darker(savedColors[i], 1);
                    if (!currentColor) {
                        savedColors[i] = (colors[i]);
                    }
                }
            }
            root.controller.sensorColors = savedColors;
        }
    }
}
