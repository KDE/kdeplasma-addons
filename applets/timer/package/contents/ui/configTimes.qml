/*
 *  SPDX-FileCopyrightText: 2020 Łukasz Korbel <corebell.it@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.12 as Kirigami
import org.kde.kcm 1.2 as KCM

ColumnLayout {
    id: timesPage
    property var cfg_predefinedTimers: []
    readonly property int maxListSize: 15

    Component.onCompleted: {
        for (var i of plasmoid.configuration.predefinedTimers) {
            timeListModel.append({"time": i})
        }
    }

    ListModel {
        id: timeListModel
        function addTimer(value) {
            timeListModel.append({"time": value})
            cfg_predefinedTimers.splice(count, 0, value)
            cfg_predefinedTimersChanged()
        }
        function removeTimer(index) {
            remove(index)
            cfg_predefinedTimers.splice(index, 1)
            cfg_predefinedTimersChanged()
        }
        function moveTimer(oldIndex, newIndex) {
            move(oldIndex, newIndex, 1)
            cfg_predefinedTimers[oldIndex] = get(oldIndex).time
            cfg_predefinedTimers[newIndex] = get(newIndex).time
            cfg_predefinedTimersChanged()
        }
        function setTimer(index, newValue) {
            setProperty(index, "time", newValue)
            cfg_predefinedTimers[index] = newValue
            cfg_predefinedTimersChanged()
        }
    }

    KCM.ScrollViewKCM {
        Layout.fillWidth: true
        Layout.fillHeight: true
        view: ListView {
            id: timeListView
            anchors.margins: 4
            model: timeListModel
            spacing: parent.spacing
            clip: true
            delegate: Kirigami.DelegateRecycler {
                width: timeListView.width
                sourceComponent: timeEditDelegate
            }
            add: Transition {
                NumberAnimation { properties: "y"; duration: Kirigami.Units.longDuration }
            }
            displaced: Transition {
                NumberAnimation { properties: "y"; duration: Kirigami.Units.longDuration }
            }

            Kirigami.PlaceholderMessage {
                visible: timeListView.count === 0
                anchors.centerIn: parent
                width: parent.width - (PlasmaCore.Units.largeSpacing * 4)
                text: i18n("If you add predefined timers here, they will appear in plasmoid context menu.");
            }
        }
    }

    QQC2.Button {
        icon.name: "list-add"
        text: i18n("Add")
        enabled: timeListModel.count < maxListSize
        onClicked: {
            timeListModel.addTimer("0")
            timeListView.positionViewAtEnd()
        }
    }

    Component {
        id: timeEditDelegate
        Kirigami.SwipeListItem {
            id: timeEditItem
            Row {
                spacing: PlasmaCore.Units.largeSpacing
                Kirigami.ListItemDragHandle {
                    anchors.verticalCenter: parent.verticalCenter
                    enabled: !editor.editable
                    listItem: timeEditItem
                    listView: timeListView
                    onMoveRequested: {
                        timeListModel.moveTimer(oldIndex, newIndex)
                    }
                }
                TimerEdit {
                    id: editor
                    alertMode: editable
                    value: time
                    property int oldValue: 0
                    onDigitModified: set(value + valueDelta)
                    function set(newValue) {
                        timeListModel.setTimer(index, (newValue).toString())
                    }
                    Component.onCompleted: editable = (value === 0)
                }
                QQC2.Label {
                    id: hintText
                    visible: editor.editable
                    text: i18n("Scroll over digits to change time")
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            actions: [
                Kirigami.Action {
                    text: i18n("Apply")
                    iconName: "dialog-ok-apply"
                    visible: editor.editable
                    onTriggered: {
                        editor.editable = false
                    }
                },
                Kirigami.Action {
                    text: i18n("Cancel")
                    iconName: "dialog-cancel"
                    visible: editor.editable
                    onTriggered: {
                        editor.editable = false
                        editor.set(editor.oldValue)
                    }
                },
                Kirigami.Action {
                    text: i18n("Edit")
                    iconName: "edit-entry"
                    visible: editor.editable === false
                    onTriggered: {
                        editor.oldValue = editor.value
                        editor.editable = true
                    }
                },
                Kirigami.Action {
                    text: i18n("Delete")
                    iconName: "entry-delete"
                    visible: editor.editable === false
                    onTriggered: {
                        timeListModel.removeTimer(index)
                    }
                }
            ]
        }
    }
}

