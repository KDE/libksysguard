/*
 *   Copyright 2019 Marco Martin <mart@kde.org>
 *   Copyright 2019 David Edmundson <davidedmundson@kde.org>
 *   Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *   Copyright 2020 David Redondo <kde@david-redondo.de>
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
import QtQuick.Controls 2.9 as QQC2
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.12 as Kirigami

import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces
import org.kde.ksysguard.formatter 1.0 as Formatter

import org.kde.quickcharts 1.0 as Charts

Faces.SensorFace {
    id: root

    readonly property int columnCount: root.controller.faceConfiguration.columnCount

    // When automatically determining the number of columns, use the square root
    // of the number of sensors, rounded up. This should give us a number of
    // columns that generally divides things evenly across the grid.
    readonly property int autoColumnCount: Math.ceil(Math.sqrt(controller.highPrioritySensorIds.length))

    // Arbitrary minimumWidth to make easier to align plasmoids in a predictable way
    Layout.minimumWidth: Kirigami.Units.gridUnit * 8
    Layout.preferredWidth: grid.preferredWidth + Kirigami.Units.largeSpacing

    contentItem: FaceGrid {
        id: grid

        columnCount: root.columnCount
        autoColumnCount: root.autoColumnCount
    }
}
