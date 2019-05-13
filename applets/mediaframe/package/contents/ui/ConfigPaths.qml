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

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.5 as Kirigami

ColumnLayout {
    id: root

    signal configurationChanged

    property var cfg_pathList: []

    function addPath(object) {
        pathModel.append( object )
        cfg_pathList.push( JSON.stringify(object) )
        configurationChanged();
    }

    function removePath(index) {
        if(pathModel.count > 0) {
            pathModel.remove(index)
            cfg_pathList.splice(index,1)
            configurationChanged();
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
        title: i18nc("@title:window", "Choose Files")
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
        title: i18nc("@title:window", "Choose a Folder")
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

    ListModel {
        id: pathModel
    }

    QQC2.ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Component.onCompleted: background.visible = true;

        ListView {
            id: pathsList

            anchors.margins: 4
            model: pathModel

            delegate: Kirigami.SwipeListItem {
                id: folderDelegate

                width: pathsList.width
                height: paintedHeight;

                QQC2.Label {
                    Layout.fillWidth: true
                    text: model.path.replace("file://", "")
                }

                actions: [
                    Kirigami.Action {
                        iconName: "list-remove"
                        tooltip: i18nd("plasma_wallpaper_org.kde.image", "Remove path")
                        onTriggered: removePath(model.index)
                    }
                ]
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true

        QQC2.Button {
            icon.name: "folder-new"
            onClicked: folderDialog.visible = true
            text: i18nc("@action:button", "Add Folder...")
        }

        QQC2.Button {
            icon.name: "document-new"
            onClicked: fileDialog.visible = true
            text: i18nc("@action:button", "Add Files...")
        }
    }
}
