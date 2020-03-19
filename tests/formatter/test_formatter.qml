/*
    Copyright (C) 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import org.kde.ksysguard.formatter 1.0 as Formatter

Pane {
    width: 400
    height: 400

    ColumnLayout {
        anchors.fill: parent

        TextField {
            id: input
            Layout.fillWidth: true
        }

        ComboBox {
            id: unitCombo

            Layout.fillWidth: true

            textRole: "key"

            model: [
                { key: "Bytes", value: Formatter.Units.UnitByte },
                { key: "Kilobytes", value: Formatter.Units.UnitKiloByte },
                { key: "Megabytes", value: Formatter.Units.UnitMegaByte },
                { key: "Bytes per Second", value: Formatter.Units.UnitByteRate },
                { key: "Hertz", value: Formatter.Units.UnitHertz },
                { key: "Second", value: Formatter.Units.UnitSecond },
                { key: "Celcius", value: Formatter.Units.UnitCelsius },
                { key: "Volt", value: Formatter.Units.UnitVolt },
                { key: "Watt", value: Formatter.Units.UnitWatt }
            ]
        }

        ComboBox {
            id: prefixCombo

            Layout.fillWidth: true

            textRole: "key"

            model: [
                { key: "None", value: Formatter.Units.MetricPrefixUnity },
                { key: "Auto-adjust", value: Formatter.Units.MetricPrefixAutoAdjust },
                { key: "Kilo", value: Formatter.Units.MetricPrefixKilo },
                { key: "Mega", value: Formatter.Units.MetricPrefixMega },
                { key: "Giga", value: Formatter.Units.MetricPrefixGiga },
                { key: "Tera", value: Formatter.Units.MetricPrefixTera },
                { key: "Peta", value: Formatter.Units.MetricPrefixPeta }
            ]
        }

        Label {
            text: Formatter.Formatter.formatValueShowNull(input.text,
                                                          unitCombo.model[unitCombo.currentIndex].value,
                                                          prefixCombo.model[prefixCombo.currentIndex].value
                                                         )
        }
    }
}
