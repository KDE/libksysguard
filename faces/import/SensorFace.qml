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
import QtQuick.Layouts 1.4

import org.kde.kirigami 2.8 as Kirigami
import org.kde.quickcharts 1.0 as Charts
import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces

Faces.AbstractSensorFace {
    id: root

    property list<Kirigami.Action> primaryActions
    property list<Kirigami.Action> secondaryActions

    implicitWidth: contentItem.implicitWidth
    implicitHeight: contentItem.implicitHeight
    Layout.minimumWidth: contentItem.Layout.minimumWidth
    Layout.minimumHeight: contentItem.Layout.minimumHeight
    Layout.preferredWidth: contentItem.Layout.preferredWidth
    Layout.preferredHeight: contentItem.Layout.preferredHeight
    Layout.maximumWidth: contentItem.Layout.maximumWidth
    Layout.maximumHeight: contentItem.Layout.maximumHeight

    property alias colorSource: colorSource

    Charts.MapProxySource {
        id: colorSource
        source: Charts.ArraySource {
            array: root.controller.highPrioritySensorIds
        }
        map: root.controller.sensorColors
    }
    Charts.ColorGradientSource {
        baseColor: Kirigami.Theme.highlightColor
        itemCount: root.controller.highPrioritySensorIds.length

        onItemCountChanged: generate()
        Component.onCompleted: generate()

        function generate() {
            //var colors = colorSource.colors;
            var savedColors = root.controller.sensorColors;
            for (var i = 0; i < root.controller.highPrioritySensorIds.length; ++i) {
                if (!savedColors.hasOwnProperty(root.controller.highPrioritySensorIds[i])) {
                    savedColors[root.controller.highPrioritySensorIds[i]] = colors[i];
                } else {
                    // Use the darker trick to make Qt validate the scring as a valid color;
                    var currentColor = Qt.darker(savedColors[root.controller.highPrioritySensorIds[i]], 1);
                    if (!currentColor) {
                        savedColors[root.controller.highPrioritySensorIds[i]] = colors[i];
                    } else {
                        savedColors[root.controller.highPrioritySensorIds[i]] = currentColor;
                    }
                }
            }
            root.controller.sensorColors = savedColors;
        }
    }
}
