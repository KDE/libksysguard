/*
 * SPDX-FileCopyrightText: 2025 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami

/**
 * A spinbox that displays a separate suffix that is not part of the value.
 *
 */
SpinBox {
    id: control

    property string suffix

    editable: true
    live: true
    spacing: Kirigami.Units.smallSpacing
    from: 0
    to: Math.pow(2, 31) - 1

    contentItem: T.TextField {
        id: textField

        implicitWidth: Math.round(contentWidth + control.leftPadding + control.spacing + suffixLabel.implicitWidth)
        implicitHeight: Math.round(contentHeight)
        font: control.font
        palette: control.palette
        text: control.textFromValue(control.value, control.locale)
        color: Kirigami.Theme.textColor
        selectionColor: Kirigami.Theme.highlightColor
        selectedTextColor: Kirigami.Theme.highlightedTextColor
        selectByMouse: true
        hoverEnabled: false // let hover events propagate to SpinBox
        verticalAlignment: Qt.AlignVCenter
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: control.inputMethodHints

        // Since the contentItem receives focus (we make them editable by default),
        // the screen reader reads its Accessible properties instead of the SpinBox's
        Accessible.name: control.Accessible.name
        Accessible.description: control.Accessible.description
    }

    Label {
        id: suffixLabel

        anchors.baseline: parent.baseline
        x: !LayoutMirroring.enabled ? Math.round(textField.contentWidth + control.leftPadding + control.spacing)
                                    : Math.round(control.width - textField.contentWidth - contentWidth - control.spacing - control.rightPadding)

        text: control.suffix
        font: control.font
        palette: control.palette
    }
}
