/*
 *  Copyright 2015  Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.1

ColumnLayout {
    id: root

    property alias cfg_interval: intervalSpinBox.value
    property alias cfg_randomize: randomizeCheckBox.checked
    property alias cfg_pauseOnMouseOver: pauseOnMouseOverCheckBox.checked
    property alias cfg_useBackground: useBackgroundCheckBox.checked
    property alias cfg_leftClickOpenImage: leftClickOpenImageCheckBox.checked
    //property alias cfg_showCountdown: showCountdownCheckBox.checked
    property alias cfg_fillMode: root.fillMode

    /*
     * Image.Stretch - the image is scaled to fit
     * Image.PreserveAspectFit - the image is scaled uniformly to fit without cropping
     * Image.PreserveAspectCrop - the image is scaled uniformly to fill, cropping if necessary
     * Image.Tile - the image is duplicated horizontally and vertically
     * Image.TileVertically - the image is stretched horizontally and tiled vertically
     * Image.TileHorizontally - the image is stretched vertically and tiled horizontally
     * Image.Pad - the image is not transformed
     */
    property int fillMode: Image.PreserveAspectFit

    GridLayout {
        columns: 2

        Label {
            Layout.row: 0
            Layout.column: 0
            Layout.alignment: Qt.AlignRight
            text: i18nc("@label:spinbox", "Change picture every")
        }

        SpinBox {
            id: intervalSpinBox
            Layout.row: 0
            Layout.column: 1
            suffix: i18nc("@item:valuesuffix spacing to number + unit (seconds)", " s")
            decimals: 1

            // Once a day should be high enough
            maximumValue: 24*(60*60)
        } // end SpinBox

        Label {
            id: fillLabel
            Layout.row: 1
            Layout.column: 0
            Layout.alignment: Qt.AlignRight
            text: i18nc("@label:listbox", "Fill mode:")
        }

        ComboBox {
            id: comboBox
            Layout.row: 1
            Layout.column: 1
            width: units.gridUnit * 10
            currentIndex: fillModeToIndex(fillMode)
            model: [
                {
                    text: i18nc("@item:inlistbox", "Stretch"),
                    description: i18nc("@info", "The image is scaled to fit the frame"),
                    value: Image.Stretch
                },
                {
                    text: i18nc("@item:inlistbox", "Preserve aspect fit"),
                    description: i18nc("@info", "The image is scaled uniformly to fit without cropping"),
                    value: Image.PreserveAspectFit
                },
                {
                    text: i18nc("@item:inlistbox", "Preserve aspect crop"),
                    description: i18nc("@info", "The image is scaled uniformly to fill, cropping if necessary"),
                    value: Image.PreserveAspectCrop
                },
                {
                    text: i18nc("@item:inlistbox", "Tile"),
                    description: i18nc("@info", "The image is duplicated horizontally and vertically"),
                    value: Image.Tile
                },
                {
                    text: i18nc("@item:inlistbox", "Tile vertically"),
                    description: i18nc("@info", "The image is stretched horizontally and tiled vertically"),
                    value: Image.TileVertically
                },
                {
                    text: i18nc("@item:inlistbox", "Tile horizontally"),
                    description: i18nc("@info", "The image is stretched vertically and tiled horizontally"),
                    value: Image.TileHorizontally
                },
                {
                    text: i18nc("@item:inlistbox", "Pad"),
                    description: i18nc("@info", "The image is not transformed"),
                    value: Image.Pad
                }
            ] // end of ComboBox model

            onActivated: root.fillMode = comboBox.model[index].value

            function fillModeToIndex(fillMode) {
                if(fillMode == Image.Stretch) {
                    return 0
                }
                else if(fillMode == Image.PreserveAspectFit) {
                    return 1
                }
                else if(fillMode == Image.PreserveAspectCrop) {
                    return 2
                }
                else if(fillMode == Image.Tile) {
                    return 3
                }
                else if(fillMode == Image.TileVertically) {
                    return 4
                }
                else if(fillMode == Image.TileHorizontally) {
                    return 5
                }
                else if(fillMode == Image.Pad) {
                    return 6
                }
            } // end of fillModeToIndex function
        } // end of ComboBox and related functions

        Label {
            id: fillModeDescription
            Layout.row: 2
            Layout.column: 1
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: comboBox.model[comboBox.currentIndex] ? comboBox.model[comboBox.currentIndex].description : ""
        }
    } // end of top section GridLayout

    // these CheckBoxes should take over as their own ColumnLayout entries
    CheckBox {
        id: randomizeCheckBox
        text: i18nc("@option:check", "Randomize items")
    }

    CheckBox {
        id: pauseOnMouseOverCheckBox
        text: i18nc("@option:check", "Pause on mouse-over")
    }

    CheckBox {
        id: useBackgroundCheckBox
        text: i18nc("@option:check", "Background frame")
    }

    CheckBox {
        id: leftClickOpenImageCheckBox
        text: i18nc("@option:check", "Left click image opens in external viewer")
    }

    Item { // tighten layout
        Layout.fillHeight: true
    }
} // end ColumnLayout
