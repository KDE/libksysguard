/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

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
    property alias cfg_lineChartStacked: stackedCheckbox.checked
    property alias cfg_lineChartFillOpacity: fillOpacitySpin.value
    property alias cfg_lineChartSmooth: smoothCheckbox.checked
    property alias cfg_showGridLines: showGridLinesCheckbox.checked
    property alias cfg_showYAxisLabels: showYAxisLabelsCheckbox.checked

    property alias cfg_rangeAutoY: rangeAutoYCheckbox.checked
    property alias cfg_rangeFromY: rangeFromYSpin.value
    property alias cfg_rangeToY: rangeToYSpin.value
    property alias cfg_rangeFromYUnit: rangeFromYSpin.unit
    property alias cfg_rangeToYUnit: rangeToYSpin.unit
    property alias cfg_rangeFromYMultiplier: rangeFromYSpin.multiplier
    property alias cfg_rangeToYMultiplier: rangeToYSpin.multiplier

    property alias cfg_historyAmount: historySpin.value

    // For backward compatibility
    property real cfg_rangeAutoX
    property real cfg_rangeFromX
    property real cfg_rangeToX

    Item {
        Kirigami.FormData.label: i18nc("@title:group", "Appearance")
        Kirigami.FormData.isSection: true
    }
    QQC2.CheckBox {
        id: showSensorsLegendCheckbox
        text: i18nc("@option:check", "Show legend")
    }
    QQC2.CheckBox {
        id: stackedCheckbox
        text: i18nc("@option:check", "Stacked charts")
    }
    QQC2.CheckBox {
        id: smoothCheckbox
        text: i18nc("@option:check", "Smooth lines")
    }
    QQC2.CheckBox {
        id: showGridLinesCheckbox
        text: i18nc("@option:check", "Show grid lines")
    }
    QQC2.CheckBox {
        id: showYAxisLabelsCheckbox
        text: i18nc("@option:check", "Show Y axis labels")
    }
    QQC2.SpinBox {
        id: fillOpacitySpin
        Kirigami.FormData.label: i18nc("@label:spinbox", "Opacity of area below line:")
        editable: true
        from: 0
        to: 100
    }
    Item {
        Kirigami.FormData.label: i18nc("title:group", "Data Ranges")
        Kirigami.FormData.isSection: true
    }
    QQC2.CheckBox {
        id: rangeAutoYCheckbox
        text: i18nc("@option:check", "Automatic Y data range")
    }
    Faces.SensorRangeSpinBox {
        id: rangeFromYSpin
        Kirigami.FormData.label: i18nc("@label:spinbox", "From (Y):")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 10
        enabled: !rangeAutoYCheckbox.checked
        sensors: controller.highPrioritySensorIds
    }
    Faces.SensorRangeSpinBox {
        id: rangeToYSpin
        Kirigami.FormData.label: i18nc("@label:spinbox", "To (Y):")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 10
        enabled: !rangeAutoYCheckbox.checked
        sensors: controller.highPrioritySensorIds
    }
    QQC2.SpinBox {
        id: historySpin
        editable: true
        from: 0
        to: Math.pow(2, 31) - 1
        Kirigami.FormData.label: i18nc("@label:spinbox", "History to show:")
        Layout.maximumWidth: Kirigami.Units.gridUnit * 10

        textFromValue: function(value, locale) {
            return i18ncp("@item:valuesuffix %1 is seconds of history", "%1 second", "%1 seconds", Number(value).toLocaleString(locale, "f", 0));
        }
        valueFromText: function(value, locale) {
            // Don't use fromLocaleString here since it will error out on extra
            // characters like the (potentially translated) seconds that gets
            // added above. Instead parseInt ignores non-numeric characters.
            return parseInt(value)
        }
    }
}

