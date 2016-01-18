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

    property int cfg_interval: 6850
    property alias cfg_randomize: randomizeCheckBox.checked
    property alias cfg_pauseOnMouseOver: pauseOnMouseOverCheckBox.checked
    property alias cfg_useBackground: useBackgroundCheckBox.checked
    property alias cfg_leftClickOpenImage: leftClickOpenImageCheckBox.checked
    property alias cfg_showCountdown: showCountdownCheckBox.checked
    
    ColumnLayout {
        
        width: parent.width
        height: parent.height

        RowLayout {
            Layout.fillWidth: true
            
            Text {
                //Layout.fillWidth: true
                
                text: i18n("Change picture every")
            }
            
            TextField {
                id: intervalTextField

                //Layout.fillWidth: true

                placeholderText: i18n("milliseconds")
                
                text: root.cfg_interval
                
                onTextChanged: {
                    cfg_interval = parseInt(text)
                }
            }
            
            Text {
                text: i18n("milliseconds")
            }
        }
        
        RowLayout {
            Layout.fillWidth: true
            
            CheckBox {
                id: randomizeCheckBox
                text: i18n("Randomize items")
                checked: true
            }
            
        }
        
        RowLayout {
            Layout.fillWidth: true
            
            CheckBox {
                id: pauseOnMouseOverCheckBox
                text: i18n("Pause on mouseover")
                checked: true
            }
            
        }
        
        RowLayout {
            Layout.fillWidth: true
            
            CheckBox {
                id: useBackgroundCheckBox
                text: i18n("Background frame")
                checked: true
            }
            
        }
        
        RowLayout {
            Layout.fillWidth: true
            
            CheckBox {
                id: leftClickOpenImageCheckBox
                text: i18n("Left click image opens in external viewer")
                checked: true
            }
            
        }
        
        RowLayout {
            Layout.fillWidth: true
            
            CheckBox {
                id: showCountdownCheckBox
                text: i18n("Show countdown")
                checked: true
            }
            
        }
        
    }
}
