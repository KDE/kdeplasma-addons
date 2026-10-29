/*
 *  SPDX-FileCopyrightText: 2020 Łukasz Korbel <corebell.it@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
    id: timesPage
    property var cfg_predefinedTimers: []
    property bool cfg_showSeconds
    readonly property int maxListSize: 15

    actions: [
        Kirigami.Action {
            icon.name: "list-add"
            text: i18nc("@action:button Add timer", "Add")
            enabled: timeListModel.count < maxListSize
            onTriggered: {
                timeListModel.addTimer("0")
                timeListView.positionViewAtEnd()
            }
        }
    ]

    // HACK: get ScrollViewKCM to draw the separator line while there's no proper API to do so
    function __headerSeparatorVisible(): bool {
        return true
    }
    extraFooterTopPadding: true

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
            timesPage.cfg_predefinedTimersChanged()
        }
        function removeTimer(index) {
            remove(index)
            cfg_predefinedTimers.splice(index, 1)
            timesPage.cfg_predefinedTimersChanged()
        }
        function moveTimer(oldIndex, newIndex) {
            move(oldIndex, newIndex, 1)
            cfg_predefinedTimers[oldIndex] = get(oldIndex).time
            cfg_predefinedTimers[newIndex] = get(newIndex).time
            timesPage.cfg_predefinedTimersChanged()
        }
        function setTimer(index, newValue) {
            setProperty(index, "time", newValue)
            cfg_predefinedTimers[index] = newValue
            timesPage.cfg_predefinedTimersChanged()
        }
    }

    view: ListView {
        id: timeListView
        anchors.margins: 4
        model: timeListModel
        spacing: parent.spacing
        clip: true
        reuseItems: true
        delegate: timeEditDelegate
        add: Transition {
            NumberAnimation { properties: "y"; duration: Kirigami.Units.longDuration }
        }
        displaced: Transition {
            NumberAnimation { properties: "y"; duration: Kirigami.Units.longDuration }
        }

        Kirigami.PlaceholderMessage {
            visible: timeListView.count === 0
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.gridUnit * 4)
            text: i18nc("@info:usagetip PlaceHolderMessage empty ListView", "If you add predefined timers here, they will appear in the widget’s context menu.");
        }
    }

    Component {
        id: timeEditDelegate
        QQC2.ItemDelegate {
            id: timeEditItem
            width: timeListView.width
            KeyNavigation.right: editor.editable ? editor : editButton
            contentItem: RowLayout {
                spacing: Kirigami.Units.gridUnit
                Kirigami.ListItemDragHandle {
                    Layout.fillHeight: true
                    enabled: !editor.editable
                    listItem: timeEditItem
                    listView: timeListView
                    onMoveRequested: (oldIndex, newIndex) => {
                        timeListModel.moveTimer(oldIndex, newIndex)
                    }
                }
                TimerEdit {
                    id: editor
                    Layout.fillWidth: true
                    alertMode: editable
                    value: time
                    showSeconds: cfg_showSeconds
                    maximumHeight: timeEditItem.availableHeight
                    property int oldValue: 0
                    onDigitModified: valueDelta => {
                        set(value + valueDelta)
                    }
                    function set(newValue) {
                        timeListModel.setTimer(index, (newValue).toString())
                    }
                    KeyNavigation.right: editable ? applyButton : editButton
                    Component.onCompleted: editable = (value === 0)
                }
                QQC2.Label {
                    id: hintText
                    visible: editor.editable
                    text: i18nc("@info:usagetip", "Scroll over digits to change time")
                    textFormat: Text.PlainText
                }
                RowLayout {
                    Layout.alignment: Qt.AlignRight

                    QQC2.ToolButton {
                        id: applyButton
                        visible: editor.editable
                        action: Kirigami.Action {
                            text: i18nc("@action:button accepts change timer", "Apply")
                            icon.name: "dialog-ok-apply"
                            onTriggered: {
                                editor.editable = false
                            }
                        }
                        KeyNavigation.right: cancelButton
                        text: action.text
                        display: QQC2.ToolButton.IconOnly
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    }

                    QQC2.ToolButton {
                        id: cancelButton
                        Layout.alignment: Qt.AlignRight
                        visible: editor.editable
                        action: Kirigami.Action {
                            text: i18nc("@action:button aborts editing timer", "Cancel")
                            icon.name: "dialog-cancel"
                            onTriggered: {
                                editor.editable = false
                                editor.set(editor.oldValue)
                            }
                        }
                        text: action.text
                        display: QQC2.ToolButton.IconOnly
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        KeyNavigation.right: deleteButton // stop here
                    }

                    QQC2.ToolButton {
                        id: editButton
                        Layout.alignment: Qt.AlignRight
                        visible: editor.editable === false
                        action: Kirigami.Action {
                            text: i18nc("@action:button enter tmer edit mode", "Edit")
                            icon.name: "edit-entry"
                            onTriggered: {
                                editor.oldValue = editor.value
                                editor.editable = true
                            }
                        }
                        text: action.text
                        display: QQC2.ToolButton.IconOnly
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        KeyNavigation.left: timeEditItem
                        KeyNavigation.right: deleteButton
                    }

                    QQC2.ToolButton {
                        id: deleteButton
                        visible: editor.editable === false
                        Layout.alignment: Qt.AlignRight
                        action: Kirigami.Action {
                            text: i18nc("@action:button delete timer", "Delete")
                            icon.name: "entry-delete"
                            onTriggered: {
                                timeListModel.removeTimer(index)
                            }
                        }
                        text: action.text
                        display: QQC2.ToolButton.IconOnly
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        KeyNavigation.right: deleteButton // stop here
                    }
                }
            }
        }
    }
}

