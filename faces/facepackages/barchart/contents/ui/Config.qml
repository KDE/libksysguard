/*
 *   Copyright 2019 Marco Martin <mart@kde.org>
 *   Copyrigth 2019 Kai Uwe Broulik <kde@broulik.de>
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
    property alias cfg_barChartStacked: stackedCheckbox.checked
    property alias cfg_showGridLines: showGridLinesCheckBox.checked
    property alias cfg_showYAxisLabels: showYAxisLabelsCheckbox.checked
    property alias cfg_rangeAuto: rangeAutoCheckbox.checked
    property alias cfg_rangeFrom: rangeFromSpin.value
    property alias cfg_rangeTo: rangeToSpin.value

    QQC2.CheckBox {
        id: showSensorsLegendCheckbox
        text: i18n("Show Sensors Legend")
    }
    QQC2.CheckBox {
        id: stackedCheckbox
        text: i18n("Stacked Bars")
    }
    QQC2.CheckBox {
        id: showGridLinesCheckBox
        text: i18n("Show Grid Lines")
    }
    QQC2.CheckBox {
        id: showYAxisLabelsCheckbox
        text: i18n("Show Y Axis Labels")
    }
    QQC2.CheckBox {
        id: rangeAutoCheckbox
        text: i18n("Automatic Data Range")
    }
    QQC2.SpinBox {
        id: rangeFromSpin
        editable: true
        from: Math.pow(-2, 31) + 1
        to: Math.pow(2, 31) - 1
        Kirigami.FormData.label: i18n("From:")
        enabled: !rangeAutoCheckbox.checked
    }
    QQC2.SpinBox {
        id: rangeToSpin
        editable: true
        from: Math.pow(-2, 31) + 1
        to: Math.pow(2, 31) - 1
        Kirigami.FormData.label: i18n("To:")
        enabled: !rangeAutoCheckbox.checked
    }
}

