/*
 *   Copyright 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
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

import QtQuick 2.12
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.2

import org.kde.kirigami 2.4 as Kirigami

import org.kde.ksysguard.formatter 1.0 as Formatter
import org.kde.ksysguard.sensors 1.0 as Sensors

ColumnLayout {
    spacing: 0

    property alias usedSensor: usedSensorObject.sensorId
    property alias totalSensor: totalSensorObject.sensorId

    Label {
        id: usedLabel
        Layout.alignment: Text.AlignHCenter;
        text: usedSensorObject.shortName;
        font: Kirigami.Theme.smallFont
        color: Kirigami.Theme.disabledTextColor
    }

    Label {
        id: usedValue
        Layout.alignment: Text.AlignHCenter;
        text: usedSensorObject.formattedValue
    }

    Kirigami.Separator { Layout.fillWidth: true }

    Label {
        id: totalValue
        Layout.alignment: Text.AlignHCenter;
        text: totalSensorObject.formattedValue
    }

    Label {
        id: totalLabel
        Layout.alignment: Text.AlignHCenter;
        text: totalSensorObject.shortName
        font: Kirigami.Theme.smallFont
        color: Kirigami.Theme.disabledTextColor
    }

    Sensors.Sensor {
        id: usedSensorObject
    }

    Sensors.Sensor {
        id: totalSensorObject
    }
}
