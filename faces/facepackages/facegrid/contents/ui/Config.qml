/*
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces

ColumnLayout {
    id: root

    property alias cfg_columnCount: columnCountSpin.value
    property string cfg_faceId

    property var faceController: controller

    Kirigami.FormLayout {
        id: form

        SpinBox {
            id: columnCountSpin
            Kirigami.FormData.label: i18n("Number of Columns:")
            editable: true
            from: 0
            to: 99999

            textFromValue: function(value, locale) {
                if (value <= 0) {
                    return i18nc("@label", "Automatic")
                }
                return value.toString()
            }

            valueFromText: function(value, locale) {
                return parseInt(value)
            }
        }

        ComboBox {
            id: faceCombo
            Kirigami.FormData.label: i18n("Display Style:")

            model: controller.availableFacesModel
            textRole: "display"
            currentIndex: {
                // TODO just make an indexOf invokable on the model?
                for (var i = 0; i < count; ++i) {
                    if (model.pluginId(i) === root.cfg_faceId) {
                        return i;
                    }
                }
                return -1;
            }
            onActivated: {
                root.cfg_faceId = model.pluginId(index);
            }
        }
    }

    Faces.FaceLoader {
        id: loader
        parentController: root.faceController
        groupName: "FaceGrid"
        faceId: root.cfg_faceId
        readOnly: false
    }

    Control {
        Layout.fillWidth: true

        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0

        contentItem: loader.controller ? loader.controller.faceConfigUi : null

        Connections {
            target: loader.controller ? loader.controller.faceConfigUi : null

            function onConfigurationChanged() {
                loader.controller.faceConfigUi.saveConfig()
                root.faceController.faceConfigUi.configurationChanged()
            }
        }
    }
}
