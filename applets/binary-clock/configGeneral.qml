/*
 *  SP9X-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
 *  SPDX-FileCopyrightText: 2018 Piotr Kąkol <piotrkakol@protonmail.com>
 *
 *  Based on analog-clock configGeneral.qml:
 *  SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QtControls
import QtQuick.Layouts
import org.kde.kquickcontrols as KQuickControls
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    property bool cfg_showOffLeds
    property alias cfg_showSeconds: showSecondsCheckBox.checked
    property alias cfg_useCustomColorForActive: useCustomColorForActiveRadioButton.checked
    property alias cfg_customColorForActive: customColorForActive.color
    property alias cfg_useCustomColorForInactive: useCustomColorForInactiveRadioButton.checked
    property alias cfg_customColorForInactive: customColorForInactive.color

    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                contentItem: QtControls.CheckBox {
                    id: showSecondsCheckBox
                    text: i18nc("@option:check", "Show Seconds")
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title:group form title for group of radiobuttons, default/custom, e.g. …Active lights", "Active Lights")

            QtControls.ButtonGroup {
                id: activeButtonGroup
            }

            Kirigami.FormEntry {
                contentItem: QtControls.RadioButton {
                    text: i18nc("@option:radio color for active lights", "Default color")
                    checked: !root.cfg_useCustomColorForActive
                    QtControls.ButtonGroup.group: activeButtonGroup
                }
            }
            Kirigami.FormEntry {
                contentItem: QtControls.RadioButton {
                    id: useCustomColorForActiveRadioButton
                    text: i18nc("@option:radio color for active lights", "Custom color")
                    QtControls.ButtonGroup.group: activeButtonGroup
                }
                trailingItems: [
                    KQuickControls.ColorButton {
                        id: customColorForActive
                        enabled: useCustomColorForActiveRadioButton.checked
                    }
                ]
            }
        }
        Kirigami.FormGroup {
            title: i18nc("@title:group form title for group of radiobuttons, default/custom/off", "Inactive lights")
            QtControls.ButtonGroup {
                id: inactiveButtonGroup
                onClicked: button => {
                    root.cfg_useCustomColorForInactive = useCustomColorForInactiveRadioButton.checked
                    root.cfg_showOffLeds = useDefaultColorForInactiveRadioButton.checked || useCustomColorForInactiveRadioButton.checked
                }
            }
            Kirigami.FormEntry {
                contentItem: QtControls.RadioButton {
                    id: useDefaultColorForInactiveRadioButton
                    QtControls.ButtonGroup.group: inactiveButtonGroup
                    checked: root.cfg_showOffLeds && !root.cfg_useCustomColorForInactive
                    text: i18nc("@option:radio color for inactive lights", "Default color")
                }
            }
            Kirigami.FormEntry {
                contentItem: QtControls.RadioButton {
                    id: useCustomColorForInactiveRadioButton
                    QtControls.ButtonGroup.group: inactiveButtonGroup
                    checked: root.cfg_showOffLeds && root.cfg_useCustomColorForInactive
                    text: i18nc("@option:radio color for inactive lights", "Custom color")
                }
                trailingItems: [
                    KQuickControls.ColorButton {
                        id:customColorForInactive
                        enabled: useCustomColorForInactiveRadioButton.checked
                    }
                ]
            }
            Kirigami.FormEntry {
                contentItem: QtControls.RadioButton {
                    id: inactiveLightsOffRadioButton
                    QtControls.ButtonGroup.group: inactiveButtonGroup
                    checked: !root.cfg_showOffLeds
                    text: i18nc("@option:radio color for inactive lights", "Off")
                }
            }
        }
    }
}
