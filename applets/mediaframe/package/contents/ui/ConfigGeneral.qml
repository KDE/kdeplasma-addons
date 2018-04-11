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

Item {
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

    ColumnLayout {    
        spacing: 20
        

        Grid {
            columns: 2
            rows: 7
            rowSpacing: 20
            columnSpacing: 10
            verticalItemAlignment: Grid.AlignVCenter
            
            // Row 1, Col 1
            Label {
                text: i18n("Change picture every")
            }

            // Row 1, Col 2
            SpinBox {
                id: intervalSpinBox
                suffix: i18n("s")
                decimals: 1

                // Once a day should be high enough
                maximumValue: 24*(60*60)
            } // end SpinBox
        
            
            // Row 2, Col 1
            Label {
                id: fillLabel
                text: i18n("Fill mode:")
            }

                
            // Row 2, Col 2  
            ComboBox {
                id: comboBox
                width: units.gridUnit * 10
                currentIndex: fillModeToIndex(fillMode)
                model: 
                    [
                         {
                            text: i18n("Stretch"),
                            value: Image.Stretch
                            },
                        
                         {
                            text: i18n("Preserve aspect fit"),
                            value: Image.PreserveAspectFit
                
                        },
                        
                         {
                            text: i18n("Preserve aspect crop"),
                            value: Image.PreserveAspectCrop
                        },
                        
                         {
                            text: i18n("Tile"),
                            value: Image.Tile
                        },

                         {
                            text: i18n("Tile vertically"),
                            value: Image.TileVertically
                        },
                        
                         {
                            text: i18n("Tile horizontally"),
                            value: Image.TileHorizontally
                        },
                        
                         {
                            text: i18n("Pad"),
                            value: Image.Pad
                        }

                    ] // end of ComboBox model
                
                    

                onCurrentIndexChanged: {
                    root.fillMode = comboBox.currentIndex
                    fillModeToIndex(root.fillMode)
                    //console.log(comboBox.currentIndex);
                }


                function fillModeToIndex(fillMode) {
                    //console.log("function called");
                    if(fillMode == Image.Stretch) {
                        fillModeDescription.text = i18n( "The image is scaled to fit the frame");
                        return 0
                    }
                    else if(fillMode == Image.PreserveAspectFit) {
                        fillModeDescription.text = i18n("The image is scaled uniformly to fit without cropping");
                        return 1
                    }
                    else if(fillMode == Image.PreserveAspectCrop) {
                        fillModeDescription.text = i18n( "The image is scaled uniformly to fill, cropping if necessary");
                        return 2
                    }
                    else if(fillMode == Image.Tile) {
                        fillModeDescription.text = i18n("The image is duplicated horizontally and vertically");
                        return 3
                    }
                    else if(fillMode == Image.TileVertically) {
                        fillModeDescription.text = i18n("The image is stretched horizontally and tiled vertically");
                        return 4
                    }
                    else if(fillMode == Image.TileHorizontally) {
                        fillModeDescription.text = i18n("The image is stretched vertically and tiled horizontally");
                        return 5
                    }
                    else if(fillMode == Image.Pad) {
                        fillModeDescription.text = i18n("The image is not transformed");
                        return 6
                    }
                } // end of fillModeToIndex function
            } // end of ComboBox and related functions
            
            
            // Row 3, Col 1 (cheater to fill empty cell)
            Label {
                width: 10
                text: ""
                } 
            

            //Row 3, Col 2
            Label {
                id: fillModeDescription
                text: i18n("The image is scaled uniformly to fit without cropping")
            }
        
        } // end of top section GridLayout
    
        // these CheckBoxes should take over as their own ColumnLayout entries
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

  } // end ColumnLayout

 } // end Item
        



