/*
 *   Copyright 2019 Marco Martin <mart@kde.org>
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
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.2 as QQC2

import org.kde.kirigami 2.8 as Kirigami

import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces

Kirigami.FormLayout {
    id: root

    property alias cfg_showLegend: showSensorsLegendCheckbox.checked
    property alias cfg_lineChartStacked: stackedCheckbox.checked
    property alias cfg_lineChartFillOpacity: fillOpacitySpin.value
    property alias cfg_lineChartSmooth: smoothCheckbox.checked

    property alias cfg_rangeAutoY: rangeAutoYCheckbox.checked
    property alias cfg_rangeFromY: rangeFromYSpin.value
    property alias cfg_rangeToY: rangeToYSpin.value
    property alias cfg_rangeAutoX: rangeAutoXCheckbox.checked
    property alias cfg_rangeFromX: rangeFromXSpin.value
    property alias cfg_rangeToX: rangeToXSpin.value

    Item {
        Kirigami.FormData.label: i18n("Appearance")
        Kirigami.FormData.isSection: true
    }
    QQC2.CheckBox {
        id: showSensorsLegendCheckbox
        text: i18n("Show Sensors Legend")
    }
    QQC2.CheckBox {
        id: stackedCheckbox
        text: i18n("Stacked Charts")
    }
    QQC2.CheckBox {
        id: smoothCheckbox
        text: i18n("Smooth Lines")
    }
    QQC2.SpinBox {
        id: fillOpacitySpin
        Kirigami.FormData.label: i18n("Fill Opacity:")
        editable: true
        from: 0
        to: 100
    }
    Item {
        Kirigami.FormData.label: i18n("Data Ranges")
        Kirigami.FormData.isSection: true
    }
    QQC2.CheckBox {
        id: rangeAutoYCheckbox
        text: i18n("Automatic Y Data Range")
    }
    QQC2.SpinBox {
        id: rangeFromYSpin
        Kirigami.FormData.label: i18n("From (Y):")
        enabled: !rangeAutoYCheckbox.checked
        editable: true
        from: -99999
        to: 99999
    }
    QQC2.SpinBox {
        id: rangeToYSpin
        editable: true
        from: -99999
        to: 99999
        Kirigami.FormData.label: i18n("To (Y):")
        enabled: !rangeAutoYCheckbox.checked
    }
    QQC2.CheckBox {
        id: rangeAutoXCheckbox
        text: i18n("Automatic X Data Range")
    }
    QQC2.SpinBox {
        id: rangeFromXSpin
        editable: true
        from: -99999
        to: 99999
        Kirigami.FormData.label: i18n("From (X):")
        enabled: !rangeAutoXCheckbox.checked
    }
    QQC2.SpinBox {
        id: rangeToXSpin
        editable: true
        from: -99999
        to: 99999
        Kirigami.FormData.label: i18n("To (X):")
        enabled: !rangeAutoXCheckbox.checked
    }
}

