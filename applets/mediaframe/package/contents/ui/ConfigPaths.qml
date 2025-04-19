/*
 *  SPDX-FileCopyrightText: 2015 Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtCore
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtQuick.Layouts

import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcmutils as KCM
import org.kde.plasma.extras as PlasmaExtras


KCM.ScrollViewKCM {

    signal configurationChanged

    property var cfg_pathList: []

    actions: [
        Kirigami.Action {
            text: i18nc("@action:button", "Add Folder…")
            icon.name: "folder-new"
            onTriggered: folderDialog.visible = true
        },
        Kirigami.Action {
            text: i18nc("@action:button", "Add Files…")
            icon.name: "document-new"
            onTriggered: fileDialog.visible = true
        }
    ]

    function addPath(object) {
        pathModel.append( object )
        cfg_pathList.push( JSON.stringify(object) )
        cfg_pathListChanged()
    }

    function removePath(index) {
        if(pathModel.count > 0) {
            pathModel.remove(index)
            cfg_pathList.splice(index,1)
            cfg_pathListChanged()
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

    view: ListView {
        id: pathsList

        model: ListModel {
            id: pathModel
        }

        delegate: Kirigami.SwipeListItem {
            id: folderDelegate

            width: pathsList.width

            contentItem: QQC2.Label {
                Layout.fillWidth: true
                text: String(model.path).replace("file://", "")
                textFormat: Text.PlainText
            }

            actions: [
                Kirigami.Action {
                    icon.name: "list-remove"
                    tooltip: i18nc("@action:button tooltip on icon-only button", "Remove path")
                    onTriggered: removePath(model.index)
                }
            ]
        }

        PlasmaExtras.PlaceholderMessage {
            anchors.centerIn: parent
            visible: pathModel.count === 0
            iconName: "viewimage-symbolic"
            text: i18nc("@info placeholdermessage if no paths set", "No media configured")
            explanation: i18nc("@info placeholdermessage explanation", "Add files or folders to display them in the widget.")
        }

        FileDialog {
            id: fileDialog

            title: i18nc("@title:window", "Choose Files")
            currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
            fileMode: FileDialog.OpenFiles

            // TODO get valid filter list from native code?
            //nameFilters: [ "Image files (*.png *.jpg)", "All files (*)" ]
            //selectedNameFilter: "All files (*)"

            onAccepted: {
                for (var i = 0; i < selectedFiles.length; ++i) {
                    var item = { 'path':selectedFiles[i], 'type':'file' }
                    addPath(item)
                }
            }
        }

        FolderDialog {
            id: folderDialog

            visible: false
            title: i18nc("@title:window", "Choose a Folder")
            currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]

            onAccepted: {
                console.log("Accepted: " + selectedFolder)

                var item = { 'path':selectedFolder, 'type':'folder' }
                addPath(item)
            }

            onRejected: {
                console.log("Canceled")
            }
        }
    }
}
