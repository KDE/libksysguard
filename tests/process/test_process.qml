/*
    Copyright (C) 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import org.kde.ksysguard.process 1.0 as Process

Pane {
    width: 400
    height: 400

    ColumnLayout {
        anchors.fill: parent

        TextField {
            id: input
            Layout.fillWidth: true
            placeholderText: "PID"
        }

        ComboBox {
            id: signalCombo

            Layout.fillWidth: true

            textRole: "key"

            model: [
                { key: "Stop", value: Process.ProcessController.StopSignal },
                { key: "Continue", value: Process.ProcessController.ContinueSignal },
                { key: "Hangup", value: Process.ProcessController.HangupSignal },
                { key: "Interrupt", value: Process.ProcessController.InterruptSignal },
                { key: "Terminate", value: Process.ProcessController.TerminateSignal },
                { key: "Kill", value: Process.ProcessController.KillSignal },
                { key: "User 1", value: Process.ProcessController.User1Signal },
                { key: "User 2", value: Process.ProcessController.User2Signal }
            ]
        }

        Button {
            Layout.fillWidth: true
            text: "Send Signal"
            onClicked: {
                var signalToSend = signalCombo.model[signalCombo.currentIndex]
                print("Sending", signalToSend.key, "(%1)".arg(signalToSend.value), "to PID", parseInt(input.text))
                var result = controller.sendSignal([parseInt(input.text)], signalToSend.value);
                print("Result:", result)
                resultLabel.text = controller.resultToString(result)
            }
        }

        Label {
            id: resultLabel
            Layout.fillWidth: true
        }
    }

    Process.ProcessController {
        id: controller
    }
}
