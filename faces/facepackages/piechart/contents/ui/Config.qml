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
    property alias cfg_fromAngle: fromAngleSpin.value
    property alias cfg_toAngle: toAngleSpin.value
    property alias cfg_smoothEnds: smoothEndsCheckbox.checked

    property alias cfg_rangeAuto: rangeAutoCheckbox.checked
    property alias cfg_rangeFrom: rangeFromSpin.value
    property alias cfg_rangeFromUnit: rangeFromSpin.unit
    property alias cfg_rangeFromMultiplier: rangeFromSpin.multiplier
    property alias cfg_rangeTo: rangeToSpin.value
    property alias cfg_rangeToUnit: rangeToSpin.unit
    property alias cfg_rangeToMultiplier: rangeToSpin.multiplier

    QQC2.CheckBox {
        id: showSensorsLegendCheckbox
        text: i18nc("@option:check", "Show legend")
    }
    QQC2.SpinBox {
        id: fromAngleSpin
        Kirigami.FormData.label: i18nc("@label:spinbox", "Starting angle:")
        from: -180
        to: 360
        editable: true
        textFromValue: function(value, locale) {
            return i18nc("@label angle degrees", "%1°", value);
        }
        valueFromText: function(text, locale) {
            return Number.fromLocaleString(locale, text.replace(i18nc("@item:intext angle degrees, used to read text from spinbox", "°"), ""));
        }
    }
    QQC2.SpinBox {
        id: toAngleSpin
        Kirigami.FormData.label: i18nc("@label:spinbox", "Total pie angle:")
        from: 0
        to: 360
        editable: true
        textFromValue: function(value, locale) {
            return i18nc("@label angle", "%1°", value);
        }
        valueFromText: function(text, locale) {
            return Number.fromLocaleString(locale, text.replace(i18nc("@item:intext angle degrees, used to read text from spinbox", "°"), ""));
        }
    }
    QQC2.CheckBox {
        id: smoothEndsCheckbox
        text: i18nc("@option:check", "Rounded lines")
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
