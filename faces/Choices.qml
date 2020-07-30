/*
 *   Copyright 2020 Marco Martin <mart@kde.org>
 *   Copyright 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
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

import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQml.Models 2.12

import org.kde.kirigami 2.12 as Kirigami
import org.kde.kitemmodels 1.0 as KItemModels
import org.kde.ksysguard.sensors 1.0 as Sensors

Control {
    id: control

    property bool supportsColors: true
    property int maxAllowedSensors: -1
    property var selected: []
    property var colors: {}

    signal selectColor(string sensorId)
    signal colorForSensorGenerated(string sensorId, color color)

    onSelectedChanged: {
        if (!control.selected) {
            return;
        }
        for (let i = 0; i < Math.min(control.selected.length, selectedModel.count); ++i) {
            selectedModel.set(i, {"sensor": control.selected[i]});
        }
        if (selectedModel.count > control.selected.length) {
            selectedModel.remove(control.selected.length, selectedModel.count - control.selected.length);
        } else if (selectedModel.count < control.selected.length) {
            for (let i = selectedModel.count; i < control.selected.length; ++i) {
                selectedModel.append({"sensor": control.selected[i]});
            }
        }
    }

    background: TextField {
        readOnly: true
        hoverEnabled: false

        onFocusChanged: {
            if (focus && (maxAllowedSensors <= 0 || repeater.count < maxAllowedSensors)) {
                popup.open()
            } else {
                popup.close()
            }
        }
        onReleased: {
            if (focus && (maxAllowedSensors <= 0 || repeater.count < maxAllowedSensors)) {
                popup.open()
            }
        }
    }

    contentItem: Flow {
        spacing: Kirigami.Units.smallSpacing

        move: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.InOutQuad
            }
        }
        Repeater {
            id: repeater
            model: ListModel {
                id: selectedModel
                function writeSelectedSensors() {
                    let newSelected = [];
                    for (let i = 0; i < count; ++i) {
                        newSelected.push(get(i).sensor);
                    }
                    control.selected = newSelected;
                    control.selectedChanged();
                }
            }

            delegate: Item {
                id: delegate
                implicitHeight: layout.implicitHeight + Kirigami.Units.smallSpacing * 2
                implicitWidth: Math.min(layout.implicitWidth + Kirigami.Units.smallSpacing * 2,
                                        control.width - control.leftPadding - control.rightPadding)
                readonly property int position: index
                Rectangle {
                    id: delegateContents
                    z: 10
                    color: Qt.rgba(
                                Kirigami.Theme.highlightColor.r,
                                Kirigami.Theme.highlightColor.g,
                                Kirigami.Theme.highlightColor.b,
                                0.25)
                    radius: Kirigami.Units.smallSpacing
                    border.color: Kirigami.Theme.highlightColor
                    border.width: 1
                    opacity: (control.maxAllowedSensors <= 0 || index < control.maxAllowedSensors) ? 1 : 0.4
                    parent: drag.active ? control : delegate

                    width: delegate.width
                    height: delegate.height
                    DragHandler {
                        id: drag
                        //TODO: uncomment as soon as we can depend from 5.15
                        //cursorShape: active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                        enabled: selectedModel.count > 1
                        onActiveChanged: {
                            if (active) {
                                let pos = delegateContents.mapFromItem(control.contentItem, 0, 0);
                                delegateContents.x = pos.x;
                                delegateContents.y = pos.y;
                            } else {
                                let pos = delegate.mapFromItem(delegateContents, 0, 0);
                                delegateContents.x = pos.x;
                                delegateContents.y = pos.y;
                                dropAnim.restart();
                                selectedModel.writeSelectedSensors();
                            }
                        }
                        xAxis {
                            minimum: 0
                            maximum: control.width - delegateContents.width
                        }
                        yAxis {
                            minimum: 0
                            maximum: control.height - delegateContents.height
                        }
                        onCentroidChanged: {
                            if (!active || control.contentItem.move.running) {
                                return;
                            }
                            let pos = control.contentItem.mapFromItem(null, drag.centroid.scenePosition.x, drag.centroid.scenePosition.y);
                            pos.x = Math.max(0, Math.min(control.contentItem.width - 1, pos.x));
                            pos.y = Math.max(0, Math.min(control.contentItem.height - 1, pos.y));

                            let child = control.contentItem.childAt(pos.x, pos.y);
                            if (child === delegate) {
                                return;
                            } else if (child) {
                                let newIndex = -1;
                                if (pos.x > child.x + child.width/2) {
                                    newIndex = Math.min(child.position + 1, selectedModel.count - 1);
                                } else {
                                    newIndex = child.position;
                                }
                                selectedModel.move(index, newIndex, 1);
                            }
                        }
                    }
                    ParallelAnimation {
                        id: dropAnim
                        XAnimator {
                            target: delegateContents
                            from: delegateContents.x
                            to: 0
                            duration: Kirigami.Units.shortDuration
                            easing.type: Easing.InOutQuad
                        }
                        YAnimator {
                            target: delegateContents
                            from: delegateContents.y
                            to: 0
                            duration: Kirigami.Units.shortDuration
                            easing.type: Easing.InOutQuad
                        }
                    }

                    Sensors.Sensor { id: sensor; sensorId: model.sensor }

                    Component.onCompleted: {
                        if (typeof control.colors === "undefined" ||
                            typeof control.colors[sensor.sensorId] === "undefined") {
                            let color = Qt.hsva(Math.random(), Kirigami.Theme.highlightColor.hsvSaturation, Kirigami.Theme.highlightColor.hsvValue, 1);
                            control.colorForSensorGenerated(sensor.sensorId, color)
                        }
                    }

                    RowLayout {
                        id: layout

                        anchors.fill: parent
                        anchors.margins: Kirigami.Units.smallSpacing

                        ToolButton {
                            visible: control.supportsColors
                            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium

                            padding: Kirigami.Units.smallSpacing

                            contentItem: Rectangle {
                                color: typeof control.colors === "undefined"  ? "black" : control.colors[sensor.sensorId]
                            }

                            onClicked: control.selectColor(sensor.sensorId)
                        }

                        Label {
                            id: label

                            Layout.fillWidth: true
                            // Have to use a separate metrics object as contentWidth will be the width of the elided text unfortunately
                            // FIXME: why +2 is necessary?
                            Layout.maximumWidth: labelMetrics.boundingRect.width + Kirigami.Units.gridUnit
                            TextMetrics {
                                id: labelMetrics
                                text: sensor.name
                                font: label.font
                            }

                            text: sensor.name
                            elide: Text.ElideRight

                            HoverHandler { id: handler }

                            ToolTip.text: sensor.name
                            ToolTip.visible: handler.hovered && label.truncated
                            ToolTip.delay: Kirigami.Units.toolTipDelay
                        }

                        ToolButton {
                            icon.name: "edit-delete-remove"
                            icon.width: Kirigami.Units.iconSizes.small
                            icon.height: Kirigami.Units.iconSizes.small
                            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium

                            onClicked: {
                                if (control.selected === undefined || control.selected === null) {
                                    control.selected = []
                                }
                                control.selected.splice(control.selected.indexOf(sensor.sensorId), 1)
                                control.selectedChanged()
                            }
                        }
                    }
                }
            }
        }

        Item { width: Kirigami.Units.iconSizes.smallMedium + Kirigami.Units.smallSpacing * 2; height: width; visible: control.maxAllowedSensors <= 0 }
    }

    Popup {
        id: popup

        // Those bindings will be immediately broken on show, but they're needed to not show the popup at a wrong position for an instant
        y: (control.Kirigami.ScenePosition.y + control.height + height > control.Window.height)
            ? - height
            : control.height
        implicitHeight: Math.min(contentItem.implicitHeight + 2, Kirigami.Units.gridUnit * 20)
        width: control.width + 2
        topMargin: 6
        bottomMargin: 6
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        modal: true
        dim: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        padding: 1

        onOpened: {
            if (control.Kirigami.ScenePosition.y + control.height + height > control.Window.height) {
                y = - height;
            } else {
                y = control.height
            }
            implicitHeight = Math.min(contentItem.implicitHeight + 2, Kirigami.Units.gridUnit * 20)
            searchField.forceActiveFocus();
        }
        onClosed: delegateModel.rootIndex = delegateModel.parentModelIndex()

        contentItem: ColumnLayout {
            spacing: 0
            RowLayout {
                Layout.fillWidth: true
                Layout.minimumHeight: implicitHeight
                Layout.maximumHeight: implicitHeight
                Layout.leftMargin: Kirigami.Units.smallSpacing
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.rightMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing

                ToolButton {
                    Layout.fillHeight: true
                    Layout.preferredWidth: height
                    icon.name: "go-previous"
                    text: i18nc("@action:button", "Back")
                    display: Button.IconOnly
                    visible: delegateModel.rootIndex.valid
                    onClicked: delegateModel.rootIndex = delegateModel.parentModelIndex()
                }

                TextField {
                    id: searchField
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    placeholderText: i18n("Search...")
                    onTextEdited: listView.searchString = text
                    KeyNavigation.down: listView
                }
            }
            Kirigami.Separator {
                Layout.fillWidth: true
            }
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ListView {
                    id: listView

                    // this causes us to load at least one delegate
                    // this is essential in guessing the contentHeight
                    // which is needed to initially resize the popup
                    cacheBuffer: 1

                    property string searchString

                    implicitHeight: contentHeight
                    model: DelegateModel {
                        id: delegateModel

                        model: listView.searchString ? sensorsSearchableModel : treeModel
                        delegate: Kirigami.BasicListItem {
                            width: listView.width
                            text: model.display
                            reserveSpaceForIcon: false

                            onClicked: {
                                if (model.SensorId.length == 0) {
                                    delegateModel.rootIndex = delegateModel.modelIndex(index);
                                } else {
                                    if (control.selected === undefined || control.selected === null) {
                                        control.selected = []
                                    }
                                    control.selected.push(model.SensorId)
                                    control.selectedChanged()
                                    popup.close()
                                }
                            }
                        }
                    }

                    Sensors.SensorTreeModel { id: treeModel }

                    KItemModels.KSortFilterProxyModel {
                        id: sensorsSearchableModel
                        filterCaseSensitivity: Qt.CaseInsensitive
                        filterString: listView.searchString
                        sourceModel: KItemModels.KSortFilterProxyModel {
                            filterRowCallback: function(row, parent) {
                                var sensorId = sourceModel.data(sourceModel.index(row, 0), Sensors.SensorTreeModel.SensorId)
                                return sensorId.length > 0
                            }
                            sourceModel: KItemModels.KDescendantsProxyModel {
                                model: listView.searchString ? treeModel : null
                            }
                        }
                    }

                    highlightRangeMode: ListView.ApplyRange
                    highlightMoveDuration: 0
                    boundsBehavior: Flickable.StopAtBounds
                }
            }
        }

        background: Item {
            anchors {
                fill: parent
                margins: -1
            }

            Kirigami.ShadowedRectangle {
                anchors.fill: parent
                anchors.margins: 1

                Kirigami.Theme.colorSet: Kirigami.Theme.View
                Kirigami.Theme.inherit: false

                radius: 2
                color: Kirigami.Theme.backgroundColor

                property color borderColor: Kirigami.Theme.textColor
                border.color: Qt.rgba(borderColor.r, borderColor.g, borderColor.b, 0.3)
                border.width: 1

                shadow.xOffset: 0
                shadow.yOffset: 2
                shadow.color: Qt.rgba(0, 0, 0, 0.3)
                shadow.size: 8
            }
        }
    }
}