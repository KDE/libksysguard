/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.9
import QtQuick.Layouts 1.2

import org.kde.kirigami 2.5 as Kirigami
import org.kde.kquickcontrols 2.0

import org.kde.ksysguard.sensors 1.0 as Sensors
import org.kde.ksysguard.faces 1.0 as Faces

Loader {
    id: root

    property Faces.SensorFaceController controller

    signal configurationChanged

    function saveConfig() {
        if (item.saveConfig) {
            item.saveConfig()
        }
        for (var key in root.controller.faceConfiguration) {
            if (item.hasOwnProperty("cfg_" + key)) {
                root.controller.faceConfiguration[key] = item["cfg_" + key]
            }
        }
    }


    onItemChanged: {
        if (!item || !root.controller.faceConfiguration) {
            return;
        }

        for (var key in root.controller.faceConfiguration) {
            if (!item.hasOwnProperty("cfg_" + key)) {
                continue;
            }

            item["cfg_" + key] = root.controller.faceConfiguration[key];
            var changedSignal = item["cfg_" + key + "Changed"];
            if (changedSignal) {
                changedSignal.connect(root.configurationChanged);
            }
        }
    }
}
