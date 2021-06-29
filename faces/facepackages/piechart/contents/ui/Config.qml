/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    property alias cfg_fromAngle: fromAngleSpin.value
    property alias cfg_toAngle: toAngleSpin.value
    property alias cfg_smoothEnds: smoothEndsCheckbox.checked
    property alias cfg_rangeAuto: rangeAutoCheckbox.checked
    property alias cfg_rangeFrom: rangeFromSpin.value
    property alias cfg_rangeTo: rangeToSpin.value

    QQC2.CheckBox {
        id: showSensorsLegendCheckbox
        text: i18n("Show Sensors Legend")
    }
    QQC2.SpinBox {
        id: fromAngleSpin
        Kirigami.FormData.label: i18n("Start from Angle")
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
        Kirigami.FormData.label: i18n("Total Pie Angle")
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
    QQC2.SpinBox {
        id: rangeFromSpin
        editable: true
        Kirigami.FormData.label: i18n("From:")
        enabled: !rangeAutoCheckbox.checked
        from: Math.pow(-2, 31) + 1
        to: Math.pow(2, 31) - 1
    }
    QQC2.SpinBox {
        id: rangeToSpin
        from: Math.pow(-2, 31) + 1
        to: Math.pow(2, 31) - 1
        editable: true
        Kirigami.FormData.label: i18n("To:")
        enabled: !rangeAutoCheckbox.checked
    }
}

