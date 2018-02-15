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

    property var cfg_pathList: []

    function addPath(object) {
        pathModel.append( object )
        cfg_pathList.push( JSON.stringify(object) )
    }

    function removePath(index) {
        if(pathModel.count > 0) {
            pathModel.remove(index)
            cfg_pathList.splice(index,1)
        }
    }

    Component.onCompleted: {
        // Load the list back in
        var list = plasmoid.configuration.pathList
        cfg_pathList = []
        for(var i in list) {
            addPath( JSON.parse(list[i]) )
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

            Button {
                iconName: "folder-new"
                onClicked: folderDialog.visible = true
                text: i18n("Add folder...")
            }

            Button {
                iconName: "document-new"
                onClicked: fileDialog.visible = true
                text: i18n("Add files...")
            }
        }

        Label {
            Layout.fillWidth: true

            text: i18n("Paths:")
        }

        ListModel {
            id: pathModel
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

                        Label {
                            id: pathText

                            Layout.fillWidth: true

                            text: model.path
                        }

                        Button {
                            id: removePathButton

                            iconName: "list-remove"

                            onClicked: removePath(model.index)
                        }
                    }
                }
            }
        }
    }
}
