/*
 * Copyright 2013  Bhushan Shah <bhush94@gmail.com>
 * Copyright 2013 Sebastian Kügler <sebas@kde.org>
 * Copyright 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 * Copyright 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.5
import QtQuick.Controls 2.5 as QtControls
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2 as QtDialogs
import org.kde.kquickcontrols 2.0 as KQC
import org.kde.kirigami 2.8 as Kirigami

Kirigami.FormLayout {
    id: appearancePage

    property alias cfg_boardSize: sizeSpinBox.value
    property alias cfg_boardColor: pieceColorPicker.color
    property alias cfg_numberColor: numberColorPicker.color
    property alias cfg_showNumerals: showNumeralsCheckBox.checked

    property alias cfg_useImage: imageBackgroundRadioButton.checked
    property alias cfg_imagePath: imagePathTextField.text

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
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    // Color background
    RowLayout {
        Kirigami.FormData.label: i18n("Background:")

        QtControls.RadioButton {
            id: colorBackgroundRadioButton
            QtControls.ButtonGroup.group: radioGroup

            text: i18n("Color:")
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
            QtControls.ButtonGroup.group: radioGroup

            text: i18n("Image:")
        }

        Kirigami.ActionTextField {
            id: imagePathTextField
            enabled: imageBackgroundRadioButton.checked

            Layout.fillWidth: true
            placeholderText: i18nc("@info:placeholder", "Path to custom image...")

            rightActions: [
                Kirigami.Action {
                    iconName: "edit-clear"
                    visible: imagePathTextField.text.length !== 0
                    onTriggered: imagePathTextField.text = "";
                }
            ]
        }

        QtControls.Button {
            enabled: imageBackgroundRadioButton.checked

            icon.name: "document-open"

            QtControls.ToolTip.visible: hovered
            QtControls.ToolTip.delay: 100
            QtControls.ToolTip.text: i18nc("@info:tooltip", "Choose image...")

            onClicked: imagePicker.open()

            QtDialogs.FileDialog {
                id: imagePicker

                title: i18nc("@title:window", "Choose an Image")

                folder: shortcuts.pictures

                // TODO ask QImageReader for supported formats
                nameFilters: [ i18n("Image Files (*.png *.jpg *.jpeg *.bmp *.svg *.svgz)") ]

                onFileUrlChanged: {
                    imagePathTextField.text = fileUrl.toString().replace("file://", "")
                }
            }
        }
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Tiles:")

        QtControls.CheckBox {
            id: showNumeralsCheckBox
            text: i18n("Colored numbers:")
        }

        KQC.ColorButton {
            id: numberColorPicker
            enabled: showNumeralsCheckBox.checked
        }
    }
}
