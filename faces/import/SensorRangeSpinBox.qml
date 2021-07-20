/*
 * SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.ksysguard.formatter 1.0 as Formatter
import org.kde.ksysguard.sensors 1.0 as Sensors

/**
 * A control to select a value with a unit.
 *
 * This is primarily intended for range selection in Face configuration pages.
 * It allows selecting a value and a unit for that value and provides that
 * value, a unit and a multiplier for that value.
 */
Control {
    id: control

    /**
     * The lower bound for the value.
     */
    property alias from: spinBox.from
    /**
     * The upper bound for the value.
     */
    property alias to: spinBox.to
    /**
     * The value.
     */
    property alias value: spinBox.value
    /**
     * The unit for the value.
     */
    property int unit
    /**
     * The multiplier to convert the provided value from its unit to the base unit.
     */
    property real multiplier
    /**
     * The list of sensors to use for retrieving unit information.
     */
    property alias sensors: unitModel.sensors
    /**
     * Emitted whenever the value, unit or multiplier changes due to user input.
     */
    signal valueModified()

    implicitWidth: leftPadding + spinBox.implicitWidth + comboBox.implicitWidth + rightPadding
    implicitHeight: topPadding + Math.max(spinBox.implicitHeight, comboBox.implicitHeight) + bottomPadding

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0
    
    contentItem: RowLayout {
        spacing: 0

        SpinBox {
            id: spinBox

            Layout.fillWidth: true
            Layout.preferredWidth: 0

            editable: true
            from: Math.pow(-2, 31) + 1
            to: Math.pow(2, 31) - 1

            onValueModified: control.valueModified()
        }

        ComboBox {
            id: comboBox

            Layout.fillWidth: true
            Layout.preferredWidth: 0

            visible: unitModel.sensors.length > 0

            textRole: "symbol"
            valueRole: "unit"

            currentIndex: 0

            onActivated: {
                control.unit = currentValue
                control.multiplier = model.data(model.index(currentIndex, 0), Sensors.SensorUnitModel.MultiplierRole)
                control.valueModified()
            }

            Component.onCompleted: updateCurrentIndex()

            model: Sensors.SensorUnitModel {
                id: unitModel
                onReadyChanged: comboBox.updateCurrentIndex()
            }

            function updateCurrentIndex() {
                if (unitModel.ready && control.unit >= 0) {
                    currentIndex = indexOfValue(control.unit)
                } else {
                    currentIndex = 0;
                }
            }
        }
    }
}
