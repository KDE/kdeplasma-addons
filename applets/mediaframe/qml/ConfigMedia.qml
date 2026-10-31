/*
 *  SPDX-FileCopyrightText: 2015 Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
pragma ComponentBehavior: Bound

import QtCore
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs

import org.kde.plasma.plasmoid
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM
import org.kde.plasma.extras as PlasmaExtras


KCM.ScrollViewKCM {
    id: mediaPage

    property list<string> cfg_pathList: []

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

    // HACK: get ScrollViewKCM to draw the separator line while there's no proper API to do so
    function __headerSeparatorVisible(): bool {
        return true
    }
    extraFooterTopPadding: true

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

    Component.onCompleted: Plasmoid.configuration.pathList.forEach( entry => {
            pathModel.append(JSON.parse(entry))
        })

    view: ListView {
        id: pathsList

        model: ListModel {
            id: pathModel
        }

        delegate: QQC2.ItemDelegate {
            id: folderDelegate

            required property string path
            required property int index

            width: pathsList.width

            Kirigami.StyleHints.useAlternateBackgroundColor: true

            contentItem: Kirigami.TitleSubtitleWithActions {
                title: folderDelegate.path.replace("file://", "")
                actions: [
                    Kirigami.Action {
                        icon.name: "list-remove"
                        text: i18nc("@action:button tooltip on icon-only button", "Remove path")
                        onTriggered: mediaPage.removePath(folderDelegate.index)
                        displayHint: Kirigami.DisplayHint.IconOnly
                        tooltip: text
                    }
                ]
            }
        }

        PlasmaExtras.PlaceholderMessage {
            anchors.centerIn: parent
            visible: pathModel.count === 0
            iconName: "insert-image-symbolic"
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
                    var item = { 'path':selectedFiles[i].toString(), 'type':'file' }
                    mediaPage.addPath(item)
                }
            }
        }

        FolderDialog {
            id: folderDialog

            visible: false
            title: i18nc("@title:window", "Choose a Folder")
            currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]

            onAccepted: {
                var item = { 'path':selectedFolder.toString(), 'type':'folder' }
                mediaPage.addPath(item)
            }
        }
    }
}
