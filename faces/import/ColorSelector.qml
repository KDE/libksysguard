/*
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kquickcontrols 2.0 as KQuickControls

/**
 * A ColorButton that allows resetting and properly deals with the lack of
 * an "invalid" state for colors in QML.
 */
Control {
    id: control

    property color color
    property color defaultColor
    property bool colorSet: false

    contentItem: RowLayout {
        KQuickControls.ColorButton {
            id: colorButton

            showAlphaChannel: true

            onAccepted: {
                control.color = color
                control.colorSet = true
            }

            Binding {
                target: colorButton
                property: "color"
                value: control.colorSet ? control.color : control.defaultColor
            }
        }

        Button {
            Layout.fillHeight: true
            Layout.preferredWidth: height

            icon.name: "edit-undo"
            display: Button.IconOnly
            flat: true

            text: i18n("Reset to default")

            enabled: control.colorSet
            onClicked: control.colorSet = false
        }
    }
}
