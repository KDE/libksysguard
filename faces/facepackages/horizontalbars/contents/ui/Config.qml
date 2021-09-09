/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

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

    property alias cfg_backgroundColorSet: backgroundSelector.colorSet
    property alias cfg_backgroundColor: backgroundSelector.color

    Faces.BackgroundColorSelector {
        id: backgroundSelector

        Kirigami.FormData.label: i18n("Background Color:")

        defaultColor: defaultBackgroundColor // From parent Loader
    }
}
