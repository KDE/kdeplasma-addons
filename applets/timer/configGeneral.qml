    /*
 *  SPDX-FileCopyrightText: 2015 Bernhard Friedreich <friesoft@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root
    property bool cfg_showTitle
    property alias cfg_title: title.text

    property bool cfg_showRemainingTime
    property bool cfg_showSeconds
    property bool cfg_showTimerToggle
    property bool cfg_showProgressBar

    property alias cfg_showNotification: showNotification.checked
    property alias cfg_notificationText: notificationText.text

    property alias cfg_runCommand: runCommand.checked
    property alias cfg_command: command.text

    property real maxComboBoxWidth: Math.max(showSeconds.implicitWidth, showRemainingTime.implicitWidth, showTimerToggle.implicitWidth, showProgressBar.implicitWidth)

    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label:textbox timer name", "Timer title")
                contentItem: QQC2.TextField {
                    id: title
                    Layout.fillWidth: true
                    placeholderText: i18nc("@info:placeholder for empty timer title", "Set a title for notification and/or widget")
                }
            }
            Kirigami.FormEntry {
                title: i18nc("@label:listbox completes e.g. … Only in tooltip", "Show seconds:")
                contentItem: QQC2.ComboBox {
                    id: showSeconds
                    Layout.preferredWidth: root.maxComboBoxWidth
                    model: [
                        i18nc("item:inlistbox completes Show seconds…", "Always"),
                        i18nc("item:inlistbox completes Show seconds…", "Only in tooltip")
                    ]
                    onActivated: index => root.cfg_showSeconds = index === 0
                    Component.onCompleted: currentIndex = root.cfg_showSeconds ? 0 : 1
                }
            }
        }
        Kirigami.FormGroup {
            title: i18nc("@title:label FormGroup title", "Appearance:")
            Kirigami.FormEntry {
                title: i18nc("@label:listbox completes e.g. … Always show", "Timer title:")
                    enabled: root.cfg_title !== ""
                contentItem: QQC2.ComboBox {
                    id: showTitle
                    Layout.preferredWidth: root.maxComboBoxWidth
                    model: [
                        i18nc("item:inlistbox completes Timer title…", "Always show"),
                        i18nc("item:inlistbox completes Timer title…", "Never show")
                    ]
                    onActivated: index => root.cfg_showTitle = index === 0
                    Component.onCompleted: currentIndex = root.cfg_showTitle ? 0 : 1
                }
            }
            Kirigami.FormEntry {
                title: i18nc("@label:listbox completes e.g. … Always show", "Progress bar:")
                contentItem: QQC2.ComboBox {
                    id: showProgressBar
                    Layout.preferredWidth: root.maxComboBoxWidth
                    model: [
                        i18nc("item:inlistbox completes Progress bar…", "Always show"),
                        i18nc("item:inlistbox completes Progress bar…", "Never show")
                    ]
                    onActivated: index => root.cfg_showProgressBar = index === 0
                    Component.onCompleted: currentIndex = root.cfg_showProgressBar ? 0 : 1
                }
            }
            Kirigami.FormEntry {
                title: i18nc("@label:listbox completes e.g. … Always show", "Remaining time:")
                contentItem: QQC2.ComboBox {
                    id: showRemainingTime
                    Layout.preferredWidth: root.maxComboBoxWidth
                    model: [
                        i18nc("item:inlistbox completes Remaining Time…", "Always show"),
                        i18nc("item:inlistbox completes Remaining Time…", "Show on large widgets and popups")
                    ]
                    onActivated: index => root.cfg_showRemainingTime = index === 0
                    Component.onCompleted: currentIndex = root.cfg_showRemainingTime ? 0 : 1
                }
            }
            Kirigami.FormEntry {
                title: i18nc("@label:listbox completes e.g. … Never show", "Timer toggle");
                contentItem: QQC2.ComboBox {
                    id: showTimerToggle
                    Layout.preferredWidth: root.maxComboBoxWidth
                    model: [
                        i18nc("item:inlistbox completes Timer toggle…", "Show on small widgets"),
                        i18nc("item:inlistbox completes Timer toggle…", "Never show")
                    ]
                    onActivated: index => root.cfg_showTimerToggle = index === 0
                    Component.onCompleted: currentIndex = root.cfg_showTimerToggle ? 0 : 1
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title:label FormGroup title", "When timer finished:")
            Kirigami.FormEntry {
                title: i18nc("@option:check prefix for checkbox with text field for notification text", "Show notification:");
                contentItem: RowLayout {
                    QQC2.CheckBox {
                        id: showNotification
                        onClicked: {
                            if (checked) {
                                notificationText.forceActiveFocus();
                            }
                        }
                    }

                    QQC2.TextField {
                        id: notificationText
                        Layout.fillWidth: true
                        enabled: showNotification.checked
                        placeholderText: i18nc("@info:placeholder default notification text if no custom notification set, keep in sync with the one in main.qml", "Timer finished")
                    }
                }
            }
            Kirigami.FormAction {
                action: Kirigami.Action {
                    enabled: showNotification.checked
                    icon.name: "preferences-desktop-notification"
                    text: i18nc("@action:button opens system settings on timer notification page", "Open notification settings")
                    onTriggered: { KCM.KCMLauncher.openSystemSettings("kcm_notifications", ["--desktop-entry", "org.kde.plasmashell", "--notifyrc", "plasma_applet_timer", "--event-id", "timerFinished"]) }
                }
            }
            Kirigami.FormEntry {
                title: i18nc("@option:check prefix for checkbox with text field for command", "Run command:")
                contentItem: RowLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.CheckBox {
                        id: runCommand
                        onClicked: {
                            if (checked) {
                                command.forceActiveFocus();
                            }
                        }
                    }

                    QQC2.TextField {
                        id: command
                        Layout.fillWidth: true
                        enabled: runCommand.checked
                        placeholderText: i18nc("@info:placeholder, only translate the message in single quotes", "kdialog --msgbox 'Timer finished'")
                    }
                }
            }
        }
    }
}
