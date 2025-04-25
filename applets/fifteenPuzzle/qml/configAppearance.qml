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

    readonly property bool unsavedChanges: cfg_showNumerals === hideNumeralsRadioButton.checked

    function saveConfig() : void {
        cfg_showNumerals = !hideNumeralsRadioButton.checked
    }

    Kirigami.FormLayout {

        QtControls.ButtonGroup {
            id: radioGroup
        }

        // Need to manually set checked state for the color button based on the
        // other onebecause it's not aliased to a saved property
        Component.onCompleted: {
            colorBackgroundRadioButton.checked = !imageBackgroundRadioButton.checked;
        }

        QtControls.SpinBox {
            id: sizeSpinBox
            Kirigami.FormData.label: i18nc("@label:spinbox", "Grid size:")
            from: 2
            to: 99
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        // Color background
        RowLayout {
            Kirigami.FormData.label: i18nc("@label:group radio button group", "Background:")

            QtControls.RadioButton {
                id: colorBackgroundRadioButton
                Layout.preferredWidth: Math.max(imageBackgroundRadioButton.implicitWidth, colorBackgroundRadioButton.implicitWidth, colorLabelsRadioButton.implicitWidth)
                QtControls.ButtonGroup.group: radioGroup

                text: i18nc("@label:chooser opens background color picker", "Color:")
            }

            KQC.ColorButton {
                id: pieceColorPicker
                enabled: colorBackgroundRadioButton.checked
            }
        }

        // Image background
        RowLayout {
            QtControls.RadioButton {
                id: imageBackgroundRadioButton
                Layout.preferredWidth: Math.max(imageBackgroundRadioButton.implicitWidth, colorBackgroundRadioButton.implicitWidth, colorLabelsRadioButton.implicitWidth)
                QtControls.ButtonGroup.group: radioGroup

                text: i18nc("@label:chooser opens background image file picker", "Image:")
            }

            Kirigami.ActionTextField {
                id: imagePathTextField
                enabled: imageBackgroundRadioButton.checked

                Layout.fillWidth: true
                placeholderText: i18nc("@info:placeholder", "Path to custom image…")

                rightActions: [
                    Kirigami.Action {
                        icon.name: "edit-clear"
                        visible: imagePathTextField.text.length !== 0
                        onTriggered: imagePathTextField.text = "";
                    }
                ]
            }

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
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QtControls.ButtonGroup {
            id: labelGroup
        }

        RowLayout {
            Kirigami.FormData.label: i18nc("@title:group formdata label", "Tile labels:")

            QtControls.RadioButton {
                id: colorLabelsRadioButton
                Layout.preferredWidth: Math.max(imageBackgroundRadioButton.implicitWidth, colorBackgroundRadioButton.implicitWidth, colorLabelsRadioButton.implicitWidth)
                QtControls.ButtonGroup.group: labelGroup
                checked: !hideNumeralsRadioButton.enabled || !hideNumeralsRadioButton.checked
                text: i18nc("@label:chooser opens label color picker", "Color:")
            }

            KQC.ColorButton {
                id: numberColorPicker
                enabled: colorLabelsRadioButton.checked
            }
        }

        QtControls.RadioButton {
            id: hideNumeralsRadioButton
            QtControls.ButtonGroup.group: labelGroup
            enabled: imageBackgroundRadioButton.checked
            checked: !root.cfg_showNumerals
            text: i18nc("@option:check", "Hide on image backgrounds")
        }
    }
}
