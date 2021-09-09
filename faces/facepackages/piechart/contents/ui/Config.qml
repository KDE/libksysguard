/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kquickcontrols 2.0 as KQuickControls

import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces

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

    property alias cfg_backgroundColorSet: backgroundSelector.colorSet
    property alias cfg_backgroundColor: backgroundSelector.color

    QQC2.CheckBox {
        id: showSensorsLegendCheckbox
        text: i18n("Show Sensors Legend")
    }
    QQC2.SpinBox {
        id: fromAngleSpin
        Kirigami.FormData.label: i18n("Start from Angle:")
        from: -180
        to: 360
        editable: true
        textFromValue: function(value, locale) {
            return i18nc("angle degrees", "%1°", value);
        }
        valueFromText: function(text, locale) {
            return Number.fromLocaleString(locale, text.replace(i18nc("angle degrees", "°"), ""));
        }
    }
    QQC2.SpinBox {
        id: toAngleSpin
        Kirigami.FormData.label: i18n("Total Pie Angle:")
        from: 0
        to: 360
        editable: true
        textFromValue: function(value, locale) {
            return i18nc("angle", "%1°", value);
        }
        valueFromText: function(text, locale) {
            return Number.fromLocaleString(locale, text.replace(i18nc("angle degrees", "°"), ""));
        }
    }
    QQC2.CheckBox {
        id: smoothEndsCheckbox
        text: i18n("Rounded Lines")
    }

    QQC2.CheckBox {
        id: rangeAutoCheckbox
        text: i18n("Automatic Data Range")
    }
    Faces.SensorRangeSpinBox {
        id: rangeFromSpin
        Kirigami.FormData.label: i18n("From:")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 10
        enabled: !rangeAutoCheckbox.checked
        sensors: controller.highPrioritySensorIds
    }
    Faces.SensorRangeSpinBox {
        id: rangeToSpin
        Kirigami.FormData.label: i18n("To:")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 10
        enabled: !rangeAutoCheckbox.checked
        sensors: controller.highPrioritySensorIds
    }

    Faces.BackgroundColorSelector {
        id: backgroundSelector

        Kirigami.FormData.label: i18n("Background Color:")

        defaultColor: defaultBackgroundColor // From parent Loader
    }
}
