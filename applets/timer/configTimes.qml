/*
 *  SPDX-FileCopyrightText: 2020 Łukasz Korbel <corebell.it@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM
import plasma.applet.org.kde.plasma.timer as TimerPlasmoid


KCM.ScrollViewKCM {
    id: timesPage
    property var cfg_predefinedTimers: []
    property bool cfg_showSeconds
    readonly property int maxListSize: 15


    actions: [
        Kirigami.Action {
            icon.name: "list-add"
            text: i18nc("@action:button Add timer", "Add")
            enabled: timeListModel.count < timesPage.maxListSize
            onTriggered: {
                timeListModel.addTimer("0")
                timeListView.positionViewAtEnd()
            }
        }
    ]

    forceHeaderSeparator: true
    forceFooterSeparator: true

    Component.onCompleted: {
        for (var i of timesPage.cfg_predefinedTimers) {
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
        spacing: timesPage.spacing
        clip: true
        reuseItems: true
        delegate: timeEditDelegate
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
        Loader {
            id: delegate
            required property int time
            required property int index

            sourceComponent: QQC2.ItemDelegate {
                id: timeEditItem

                focus: true
                width: timeListView.width
                Kirigami.Theme.useAlternateBackgroundColor: true
                KeyNavigation.right: editor.editable ? hoursSpinBox : editButton
                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Kirigami.ListItemDragHandle {
                        Layout.fillHeight: true
                        enabled: !editor.editable
                        listItem: timeEditItem
                        listView: timeListView
                        onMoveRequested: (oldIndex, newIndex) => {
                            timeListModel.moveTimer(oldIndex, newIndex)
                        }
                    }
                    Kirigami.Heading {
                        id: timeLabel
                        Layout.fillWidth: true
                        visible: !editor.editable
                        text: TimerPlasmoid.Timer.secondsToString(delegate.time, "hh:mm:ss")
                    }
                    RowLayout {
                        id: editor

                        property bool editable: false
                        property int oldValue: delegate.time
                        readonly property int value: hoursSpinBox.value * (60*60) + minutesSpinBox.value * 60 + secondsSpinBox.value

                        Layout.fillWidth: true
                        visible: editable

                        Component.onCompleted: {
                            set(oldValue)
                            editable = oldValue === 0
                        }

                        function set(newValue: int) : void {
                            const secondsPerHour = 60*60
                            hoursSpinBox.value = Math.floor(newValue / secondsPerHour)
                            minutesSpinBox.value = Math.floor((newValue % secondsPerHour) / 60)
                            secondsSpinBox.value = newValue % 60
                        }

                        QQC2.SpinBox {
                            id: hoursSpinBox
                            from: 0
                            to: 23
                            textFromValue: (value) => value.toString().padStart(2, "0")
                            KeyNavigation.right: minutesSpinBox
                        }
                        Kirigami.Heading { text: ":" }
                        QQC2.SpinBox {
                            id: minutesSpinBox
                            from: -1
                            to: 60
                            textFromValue: (value) => value.toString().padStart(2, "0")
                            onValueChanged: {
                                if (value == 60) {
                                    value = hoursSpinBox.value < 23 ? 0 : 59
                                    hoursSpinBox.increase()
                                } else if (value == -1) {
                                    value = hoursSpinBox.value > 0 ? 59 : 0
                                    hoursSpinBox.decrease()
                                }
                            }
                            KeyNavigation.right: secondsSpinBox
                        }
                        Kirigami.Heading { text: ":" }
                        QQC2.SpinBox {
                            id: secondsSpinBox
                            from: -1
                            to: 60
                            textFromValue: (value) => value.toString().padStart(2, "0")
                            onValueChanged: {
                                if (value == 60) {
                                    value = (hoursSpinBox.value < 23 || minutesSpinBox.value < 60) ? 0 : 59
                                    minutesSpinBox.increase()
                                } else if (value == -1) {
                                    value = (hoursSpinBox.value > 0 || minutesSpinBox.value > 0)? 59 : 0
                                    minutesSpinBox.decrease()
                                }
                            }
                            KeyNavigation.right: applyButton
                        }
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignRight

                        QQC2.Button {
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

                        QQC2.Button {
                            id: cancelButton
                            Layout.alignment: Qt.AlignRight
                            visible: editor.editable
                            action: Kirigami.Action {
                                text: i18nc("@action:button aborts editing timer", "Cancel")
                                icon.name: "dialog-cancel"
                                onTriggered: {
                                    editor.editable = false
                                }
                            }
                            text: action.text
                            display: QQC2.ToolButton.IconOnly
                            QQC2.ToolTip.text: text
                            QQC2.ToolTip.visible: hovered
                            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                            KeyNavigation.right: deleteButton // stop here
                        }

                        QQC2.Button {
                            id: editButton
                            Layout.alignment: Qt.AlignRight
                            visible: editor.editable === false
                            action: Kirigami.Action {
                                text: i18nc("@action:button enter tmer edit mode", "Edit")
                                icon.name: "edit-entry"
                                onTriggered: {
                                    editor.oldValue = delegate.time
                                    editor.set(editor.oldValue)
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

                        QQC2.Button {
                            id: deleteButton
                            visible: editor.editable === false
                            Layout.alignment: Qt.AlignRight
                            action: Kirigami.Action {
                                text: i18nc("@action:button delete timer", "Delete")
                                icon.name: "entry-delete"
                                onTriggered: {
                                    timeListModel.removeTimer(delegate.index)
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
}

