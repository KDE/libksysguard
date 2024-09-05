/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

Kirigami.FormLayout {
    id: root

    property alias cfg_showLegend: showSensorsLegendCheckbox.checked
    property alias cfg_barChartStacked: stackedCheckbox.checked
    property alias cfg_showGridLines: showGridLinesCheckBox.checked
    property alias cfg_horizontalBars: horizontalBarsRadio.checked
    property alias cfg_showYAxisLabels: showYAxisLabelsCheckbox.checked

    property alias cfg_rangeAuto: rangeAutoCheckbox.checked
    property alias cfg_rangeFrom: rangeFromSpin.value
    property alias cfg_rangeFromUnit: rangeFromSpin.unit
    property alias cfg_rangeFromMultiplier: rangeFromSpin.multiplier
    property alias cfg_rangeTo: rangeToSpin.value
    property alias cfg_rangeToUnit: rangeToSpin.unit
    property alias cfg_rangeToMultiplier: rangeToSpin.multiplier

    QQC2.CheckBox {
        Kirigami.FormData.label: i18nc("@title:group General options", "General:")
        id: showSensorsLegendCheckbox
        text: i18nc("@option:check", "Show legend")
    }
    QQC2.CheckBox {
        id: stackedCheckbox
        text: i18nc("@option:check", "Stacked bars")
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    QQC2.RadioButton {
        Kirigami.FormData.label: i18nc("@label for radiobutton group, Orientation of bar chart", "Bar orientation:")
        id: verticalBarsRadio
        text: i18nc("@option:radio Vertical option for the bar chart", "Vertical")
        checked: !cfg_horizontalBars
    }
    QQC2.RadioButton {
        id: horizontalBarsRadio
        text: i18nc("@option:radio Horizontal option for the bar chart", "Horizontal")
        onClicked: {
            showYAxisLabelsCheckbox.checked = false
        }
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    QQC2.CheckBox {
        Kirigami.FormData.label: i18nc("@label Options about axis and label of bar chart", "Axis:")
        id: showYAxisLabelsCheckbox
        text: i18nc("@option:check", "Show Y axis labels")
        enabled: !horizontalBarsRadio.checked
    }
    QQC2.CheckBox {
        id: showGridLinesCheckBox
        text: i18nc("@option:check", "Show grid lines")
    }
    QQC2.CheckBox {
        id: rangeAutoCheckbox
        text: i18nc("@option:check", "Automatic data range")
    }
    Faces.SensorRangeSpinBox {
        id: rangeFromSpin
        Kirigami.FormData.label: i18nc("@label:spinbox data range", "From:")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 10
        enabled: !rangeAutoCheckbox.checked
        sensors: controller.highPrioritySensorIds
    }
    Faces.SensorRangeSpinBox {
        id: rangeToSpin
        Kirigami.FormData.label: i18nc("@label:spinbox data range", "To:")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 10
        enabled: !rangeAutoCheckbox.checked
        sensors: controller.highPrioritySensorIds
    }
}

