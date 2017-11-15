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

import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.kquickcontrolsaddons 2.0 as KQuickAddons

Item {
    id: root
    width: parent.width
    height: parent.height

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

    ColumnLayout {

        width: parent.width
        height: parent.height

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: i18n("Change picture every")
            }

            SpinBox {

                id: intervalSpinBox

                suffix: i18n("s")
                decimals: 1

                // Once a day should be high enough
                maximumValue: 24*(60*60)
            }
        }

        ColumnLayout {
            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: i18n("Fill mode:")
                }

                ComboBox {
                    id: comboBox
                    currentIndex: fillModeToIndex(fillMode)
                    model:
                        [
                            {
                                "text": i18n("Stretch"),
                                "value": Image.Stretch,
                                "description": i18n("The image is scaled to fit")
                            },
                            {
                                "text": i18n("Preserve aspect fit"),
                                "value": Image.PreserveAspectFit,
                                "description": i18n("The image is scaled uniformly to fit without cropping")
                            },
                            {
                                "text": i18n("Preserve aspect crop"),
                                "value": Image.PreserveAspectCrop,
                                "description": i18n("The image is scaled uniformly to fill, cropping if necessary")
                            },
                            {
                                "text": i18n("Tile"),
                                "value": Image.Tile,
                                "description": i18n("The image is duplicated horizontally and vertically")
                            },
                            {
                                "text": i18n("Tile vertically"),
                                "value": Image.TileVertically,
                                "description": i18n("The image is stretched horizontally and tiled vertically")
                            },
                            {
                                "text": i18n("Tile horizontally"),
                                "value": Image.TileHorizontally,
                                "description": i18n("The image is stretched vertically and tiled horizontally")
                            },
                            {
                                "text": i18n("Pad"),
                                "value": Image.Pad,
                                "description": i18n("The image is not transformed")
                            }
                        ]

                    onActivated: root.fillMode = comboBoxItems.get(index).value

                    onCurrentIndexChanged: fillModeDescription.text = comboBoxItems.get(currentIndex).description

                    function fillModeToIndex(fillMode) {
                        if(fillMode == Image.Stretch)
                            return 0
                        else if(fillMode == Image.PreserveAspectFit)
                            return 1
                        else if(fillMode == Image.PreserveAspectCrop)
                            return 2
                        else if(fillMode == Image.Tile)
                            return 3
                        else if(fillMode == Image.TileVertically)
                            return 4
                        else if(fillMode == Image.TileHorizontally)
                            return 5
                        else if(fillMode == Image.Pad)
                            return 6
                    }
                }
            }
            Label {
                id: fillModeDescription
                text: i18n("The image is scaled uniformly to fit without cropping")
            }
        }

        CheckBox {
            id: randomizeCheckBox
            text: i18n("Randomize items")
        }

        CheckBox {
            id: pauseOnMouseOverCheckBox
            text: i18n("Pause on mouseover")
        }

        CheckBox {
            id: useBackgroundCheckBox
            text: i18n("Background frame")
        }

        CheckBox {
            id: leftClickOpenImageCheckBox
            text: i18n("Left click image opens in external viewer")
        }

        /*
        CheckBox {
            id: showCountdownCheckBox
            text: i18n("Show countdown")
        }
        */

    }
}
