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
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.kquickcontrolsaddons 2.0 as KQuickAddons

Item {
    id: root
    width: parent.width
    height: parent.height

    property string cfg_pathList: ""
    
    function addPath(object) {
        pathModel.append( object )
        
        var t = []
        if(cfg_pathList != '')
            t = JSON.parse(cfg_pathList)
        t.push(object)
        cfg_pathList = JSON.stringify(t)
    }
    
    function removePath(index) {
        if(pathModel.count > 0) {
            pathModel.remove(index)
            var t = JSON.parse(cfg_pathList)
            t.splice(index,1)
            cfg_pathList = JSON.stringify(t)
        }
    }
    
    FileDialog {
        id: fileDialog
        
        visible: false
        
        title: i18n("Choose files")
        
        folder: shortcuts.pictures
        
        selectMultiple: true
        
        // TODO get valid filter list from native code?
        //nameFilters: [ "Image files (*.png *.jpg)", "All files (*)" ]
        //selectedNameFilter: "All files (*)"
        
        onAccepted: {
            console.log("Accepted: " + fileUrls)
            
            for (var i = 0; i < fileUrls.length; ++i) {
                var item = { 'path':fileUrls[i], 'type':'file' }
                addPath(item)
            }
        }
        
        onRejected: {
            console.log("Canceled")
        }
    }
    
    FileDialog {
        id: folderDialog
        
        visible: false
        
        title: i18n("Choose a folder")
        
        folder: shortcuts.pictures
        
        selectFolder: true
        
        onAccepted: {
            console.log("Accepted: " + fileUrls)
            
            for (var i = 0; i < fileUrls.length; ++i) {
                var item = { 'path':fileUrls[i], 'type':'folder' }
                addPath(item)
            }
        }
        
        onRejected: {
            console.log("Canceled")
        }
        
    }
    
    ColumnLayout {
        
        width: parent.width
        height: parent.height
        
        RowLayout {
            Layout.fillWidth: true
            
            PlasmaComponents.Button {
                iconSource: "folder"
                onClicked: folderDialog.visible = true
            }
            
            Text {
                text: i18n("Add folder")
            }
            
            PlasmaComponents.Button {
                iconSource: "document-new"
                onClicked: fileDialog.visible = true
            }
            
            Text {
                text: i18n("Add files")
            }
            
        }
        
        Label {
            Layout.fillWidth: true

            text: i18n("Paths")
        }
        
        ListModel { 
            id: pathModel
            
            Component.onCompleted: {
                var json = plasmoid.configuration.pathList
                if(json != '') {
                    var list = JSON.parse(json)
                    for(var i in list) {
                        var pathObject = list[i]
                        addPath(pathObject)
                    }
                }
                
            }
        }
        
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                frameVisible: true

                ListView {
                    width: parent.width
                    model: pathModel

                    delegate: RowLayout {
                        width: parent.width
                        
                        Text {
                            id: pathText

                            Layout.fillWidth: true
                            
                            text: model.path
                        }
                        
                        PlasmaComponents.Button {
                            id: removePathButton

                            iconSource: "list-remove"
                            
                            onClicked: removePath(model.index)
                        }
                    }
                }
                
            }

        }
    }
}
