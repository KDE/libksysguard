/*
 *   Copyright 2019 Marco Martin <mart@kde.org>
 *   Copyright 2019 David Edmundson <davidedmundson@kde.org>
 *   Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
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
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.2 as QQC2

import org.kde.kirigami 2.5 as Kirigami
import org.kde.kquickcontrols 2.0
import org.kde.kconfig 1.0 // for KAuthorized
import org.kde.newstuff 1.62 as NewStuff

import org.kde.quickcharts 1.0 as Charts
import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces

Kirigami.FormLayout {
    id: root

    signal configurationChanged

    function saveConfig() {
        controller.title = cfg_title;
        controller.faceId = cfg_chartFace;

        var preset = pendingPreset;
        pendingPreset = "";
        if (preset != "") {
            controller.loadPreset(preset);
            root.controller.highPrioritySensorColors = automaticColorSource.colors
        }
    }

    property Faces.SensorFaceController controller
    property alias cfg_title: titleField.text
    property string cfg_chartFace

    onCfg_titleChanged: configurationChanged();
    onCfg_chartFaceChanged: configurationChanged();

    // config keys of the selected preset to be applied on save
    property string pendingPreset

    Component.onCompleted: {
        cfg_title = controller.title;
        cfg_chartFace = controller.faceId;
    }

    Charts.ColorGradientSource {
        id: automaticColorSource
        baseColor: Kirigami.Theme.highlightColor
        itemCount: root.controller.highPrioritySensorIds.length
    }

    Kirigami.OverlaySheet {
        id: presetSheet
        parent: root
        ListView {
            implicitWidth: Kirigami.Units.gridUnit * 15
            model: controller.availablePresetsModel
            delegate: Kirigami.SwipeListItem {
                contentItem: QQC2.Label {
                    Layout.fillWidth: true
                    text: model.display
                }
                actions: Kirigami.Action {
                    icon.name: "delete"
                    visible: model.writable
                    onTriggered: controller.uninstallPreset(model.pluginId);
                }
                onClicked: {
                    cfg_title = model.display;
                    pendingPreset = model.pluginId;
                    if (model.config.chartFace) {
                        cfg_chartFace = model.config.chartFace;
                    }

                    root.configurationChanged();
                    presetSheet.close();
                }
            }
        }
    }
    RowLayout {
        Kirigami.FormData.label: i18nd("KSysGuardSensorFaces", "Presets:")
        
        QQC2.Button {
            icon.name: "document-open"
            text: i18nd("KSysGuardSensorFaces", "Load Preset...")
            onClicked: presetSheet.open()
        }

        NewStuff.Button {
            Accessible.name: i18nd("KSysGuardSensorFaces", "Get new presets...")
            configFile: "systemmonitor-presets.knsrc"
            text: ""
            onChangedEntriesChanged: controller.availablePresetsModel.reload();
            QQC2.ToolTip {
                text: parent.Accessible.name
            }
        }

        QQC2.Button {
            id: saveButton
            icon.name: "document-save"
            text: i18nd("KSysGuardSensorFaces", "Save Settings As Preset")
            enabled: controller.currentPreset.length == 0
            onClicked: controller.savePreset();
        }
    }

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
    }

    QQC2.TextField {
        id: titleField
        Kirigami.FormData.label: i18nd("KSysGuardSensorFaces", "Title:")
    }

    RowLayout {
        Kirigami.FormData.label: i18nd("KSysGuardSensorFaces", "Display Style:")
        QQC2.ComboBox {
            id: faceCombo
            model: controller.availableFacesModel
            textRole: "display"
            currentIndex: {
                // TODO just make an indexOf invokable on the model?
                for (var i = 0; i < count; ++i) {
                    if (model.pluginId(i) === cfg_chartFace) {
                        return i;
                    }
                }
                return -1;
            }
            onActivated: {
                cfg_chartFace = model.pluginId(index);
            }
        }

        NewStuff.Button {
            text: i18nd("KSysGuardSensorFaces", "Get New Display Styles...")
            configFile: "systemmonitor-faces.knsrc"
            onChangedEntriesChanged: controller.availableFacesModel.reload();
        }
    }
}
