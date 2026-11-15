/*
 * SPDX-FileCopyrightText: 2013 Bhushan Shah <bhush94@gmail.com>
 * SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 * SPDX-FileCopyrightText: 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtCore
import QtQuick.Controls as QtControls
import QtQuick.Layouts
import QtQuick.Dialogs as QtDialogs

import org.kde.kquickcontrols as KQC
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root
    property alias cfg_boardSize: sizeSpinBox.value
    property alias cfg_boardColor: pieceColorPicker.color
    property alias cfg_numberColor: numberColorPicker.color
    property bool  cfg_showNumerals

    property alias cfg_useImage: imageBackgroundRadioButton.checked
    property alias cfg_imagePath: imagePathTextField.text

    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label:spinbox", "Grid size:")
                contentItem: QtControls.SpinBox {
                    id: sizeSpinBox
                    from: 2
                    to: 99
                }
            }
        }
        Kirigami.FormGroup {
            QtControls.ButtonGroup {
                id: radioGroup
            }
            Kirigami.FormEntry {
                title: i18nc("@label:group radio button group", "Background:")
                contentItem: QtControls.RadioButton {
                    id: colorBackgroundRadioButton
                    Layout.preferredWidth: Math.max(imageBackgroundRadioButton.implicitWidth, colorBackgroundRadioButton.implicitWidth, colorLabelsRadioButton.implicitWidth)
                    QtControls.ButtonGroup.group: radioGroup
                    checked: !imageBackgroundRadioButton.checked
                    text: i18nc("@label:chooser opens background color picker", "Color")
                }
                trailingItems: [
                    KQC.ColorButton {
                        id: pieceColorPicker
                        enabled: colorBackgroundRadioButton.checked
                    }
                ]
            }
            Kirigami.FormEntry {
                contentItem: QtControls.RadioButton {
                    id: imageBackgroundRadioButton
                    Layout.preferredWidth: Math.max(imageBackgroundRadioButton.implicitWidth, colorBackgroundRadioButton.implicitWidth, colorLabelsRadioButton.implicitWidth)
                    QtControls.ButtonGroup.group: radioGroup

                    text: i18nc("@label:chooser opens background image file picker", "Image")
                }
                trailingItems: [
                    Kirigami.ActionTextField {
                        id: imagePathTextField
                        enabled: imageBackgroundRadioButton.checked
                        onEnabledChanged: if (enabled) { forceActiveFocus(Qt.OtherFocusReason) }

                        Layout.leftMargin: Kirigami.Units.smallSpacing
                        Layout.fillWidth: true
                        placeholderText: i18nc("@info:placeholder", "Path to custom image…")

                        rightActions: [
                            Kirigami.Action {
                                icon.name: "edit-clear"
                                visible: imagePathTextField.text.length !== 0
                                onTriggered: imagePathTextField.text = "";
                            }
                        ]
                    },

                    QtControls.Button {
                        id: imageButton
                        enabled: imageBackgroundRadioButton.checked

                        icon.name: "document-open"
                        text: i18nc("@action:button", "Choose image…")
                        display: QtControls.Button.IconOnly
                        QtControls.ToolTip.text: text
                        QtControls.ToolTip.visible: hovered
                        QtControls.ToolTip.delay: Kirigami.Units.toolTipDelay

                        onClicked: imagePicker.open()

                        QtDialogs.FileDialog {
                            id: imagePicker

                            title: i18nc("@title:window", "Choose an Image")

                            currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]

                            // TODO ask QImageReader for supported formats
                            nameFilters: [ i18nc("@item:inlistbox file type category for file picker", "Image Files (*.png *.jpg *.jpeg *.bmp *.svg *.svgz)") ]

                            onAccepted: {
                                imagePathTextField.text = selectedFile.toString().replace("file://", "")
                            }
                        }
                    }
                ]
            }
        }

        Kirigami.FormGroup {
            QtControls.ButtonGroup {
                id: labelGroup
                onClicked: root.cfg_showNumerals = !hideNumeralsRadioButton.checked
            }
            Kirigami.FormEntry {
                title: i18nc("@title:group formdata label", "Tile labels:")
                contentItem: QtControls.RadioButton {
                    id: colorLabelsRadioButton
                    Layout.preferredWidth: Math.max(imageBackgroundRadioButton.implicitWidth, colorBackgroundRadioButton.implicitWidth, colorLabelsRadioButton.implicitWidth)
                    QtControls.ButtonGroup.group: labelGroup
                    checked: !hideNumeralsRadioButton.enabled || !hideNumeralsRadioButton.checked
                    text: i18nc("@label:chooser opens label color picker", "Color")
                }
                trailingItems: [
                    KQC.ColorButton {
                        id: numberColorPicker
                        enabled: colorLabelsRadioButton.checked
                    }
                ]
            }
            Kirigami.FormEntry {
                contentItem: QtControls.RadioButton {
                    id: hideNumeralsRadioButton
                    QtControls.ButtonGroup.group: labelGroup
                    enabled: imageBackgroundRadioButton.checked
                    checked: !root.cfg_showNumerals
                    text: i18nc("@option:check Hide tile labels", "Hide on image backgrounds")
                }
            }
        }
    }
}
