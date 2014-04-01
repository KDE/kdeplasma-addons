/***************************************************************************
 *   Copyright (C) 2013 by Aleix Pol Gonzalez <aleixpol@blue-systems.com>  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as QtExtra

DropArea {
    id: root

    property string url: ""
    property bool properlySent: true
    property QtObject lastJob: null

    function findMimeType(mimeName, data) {
        if (mimeName == "text/uri-list")
            return mimeDb.mimeTypeForUrl(data[0]);
        else
            return mimeDb.mimeTypeForName(mimeName);
    }

    function preferredSourceForMimetypes(mimeNames, data) {
        for(var i in mimeNames) {
            var mime = findMimeType(mimeNames[i], data);
            var category = mime.name.substr(0, mime.name.indexOf("/"));

            return { "mime": mime, "format": mimeNames[i], "source": plasmoid.configuration[category] };
        }
        return {}
    }

    onEntered: {
        var source = preferredSourceForMimetypes(drag.formats, [drag.getDataAsString("text/uri-list")]);

        icon.source = source.mime.iconName;
        drag.accept();
    }

    QtExtra.Clipboard {
        id: clipboard
    }

    QtExtra.MimeDatabase {
        id: mimeDb
    }

    Component {
        id: menuItemComponent
        PasteMenuItem {}
    }

    function sendData(source, data) {
        var service = shareDataSource.serviceForSource(source);
        var operation = service.operationDescription("share");
        operation.content = data;
        root.lastJob = service.startOperationCall(operation);
        root.lastJob.finished.connect(function(){
            root.state = root.lastJob.error==0 ? "success" : "failure";
            var resultUrl = root.lastJob.result;

            if (root.lastJob.error==0) {
                root.url = resultUrl;
                clipboard.content = root.url;

                menuItemComponent.createObject(menu, {"text": root.url});
                if(menu.content.length >= 2+plasmoid.configuration.historySize) {
                    menu.at(2).deleteLater()
                }
            }
        });
        root.state = "sending";
    }

    onDropped: {
        var pref = preferredSourceForMimetypes(drop.formats);
        sendData(pref.source, drop.getDataAsString(pref.format))
        drop.accept();
    }

    onExited: icon.source = "edit-paste"

    PlasmaComponents.ContextMenu {
        id: menu
        visualParent: parent

        PlasmaComponents.MenuItem {
            text: i18n("Paste")
            icon: "edit-paste"

            onClicked: {
                var pref = preferredSourceForMimetypes(clipboard.formats, clipboard.contentFormat("text/uri-list"));

                sendData(pref.source, clipboard.contentFormat(pref.format));
            }
        }

        PlasmaComponents.MenuItem { separator: true }
    }

    PlasmaCore.Dialog {
        id: tooltipDialog
        visualParent: parent

        mainItem: Item {
            width: 300
            height: 100

            PlasmaCore.IconItem {
                id: tooltipIcon
                width: parent.width*.25
                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                }
                source: "edit-paste"
            }

            PlasmaComponents.Label {
                id: tooltipText
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter

                onLinkActivated: Qt.openUrlExternally(link)
                anchors {
                    left: tooltipIcon.right
                    right: parent.right
                    bottom: parent.bottom
                    top: parent.top
                }
            }
        }
    }

    PlasmaCore.IconItem {
        id: icon
        anchors.fill: parent
    }

    PlasmaComponents.BusyIndicator {
        id: busy
        anchors.fill: parent
        visible: false
    }

    Rectangle {
        border {
            color: theme.textColor
            width: root.containsDrag ? 5 : 1
        }
        color: theme.backgroundColor
        radius: 35
        anchors.fill: parent
        opacity: root.containsDrag || mouseArea.containsMouse ? .3 : 0

        Behavior on opacity { NumberAnimation { duration: 100 } }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.RightButton

        onClicked: {
            menu.open()
        }
    }

    PlasmaCore.DataSource {
        id: shareDataSource
        engine: "org.kde.plasma.dataengine.share"
        connectedSources: sources
    }

    state: "idle"
    states: [
        State {
            name: "idle"
            PropertyChanges { target: icon; source: "edit-paste" }
            PropertyChanges { target: tooltipIcon; source: "edit-paste" }
            PropertyChanges { target: tooltipText; text: i18n("Drop text or an image onto me to upload it to an online service.") }
        },
        State {
            name: "sending"
            PropertyChanges { target: icon; visible: false }
            PropertyChanges { target: busy; visible: true }
            PropertyChanges { target: busy; running: true }

            PropertyChanges { target: tooltipIcon; source: "view-history" }
            PropertyChanges { target: tooltipText; text: i18n("Sending...") }
            PropertyChanges { target: tooltipDialog; visible: true }
        },
        State {
            name: "success"
            PropertyChanges { target: icon; source: "dialog-ok" }

            PropertyChanges { target: tooltipIcon; source: "dialog-ok" }
            PropertyChanges { target: tooltipText; text: i18n("Successfully uploaded to <a href='%1'>%1</a>.", root.url) }
            PropertyChanges { target: tooltipDialog; visible: true }
        },
        State {
            name: "failure"
            PropertyChanges { target: icon; source: "dialog-cancel" }

            PropertyChanges { target: tooltipIcon; source: icon.source }
            PropertyChanges { target: tooltipText; text: i18n("Error during upload. Try again.") }
            PropertyChanges { target: tooltipDialog; visible: true }
        }
    ]
}
