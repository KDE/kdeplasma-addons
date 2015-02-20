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
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as QtExtra
import QtQuick.Layouts 1.1
import org.kde.plasma.private.purpose 1.0
import org.kde.draganddrop 2.0 as DragDrop

DragDrop.DropArea {
    id: root

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Layout.fillWidth: false
    Layout.fillHeight:false
    Layout.minimumWidth: 0
    Layout.minimumHeight: 0

    property string url: ""
    property bool properlySent: true
    property QtObject lastJob: null

    function findMimeType(mimename, data) {
        if (mimename == "text/uri-list")
            return mimeDb.mimeTypeForUrl(data[0]);
        else {
            return mimeDb.mimeTypeForName(mimename);
        }
    }

    onDragEnter: {
        var mimetype = firstMimeType(event.mimeData.formats);
        var source = findMimeType(mimetype, [event.mimeData.getDataAsByteArray("text/uri-list")]);

        icon.source = source.iconName;
        event.accepted = true
    }
    onDragLeave: icon.source = "edit-paste"

    QtExtra.Clipboard {
        id: clipboard
    }

    QtExtra.MimeDatabase {
        id: mimeDb
    }

    function resetActions() {
        for(var v in root.pasteUrls) {
            plasmoid.setAction(v, root.pasteUrls[v], "");
        }
    }

    ShareDialog {
        id: shareDialog
        location: plasmoid.location
        inputData: { urls: [] }
        visualParent: parent
        onRunningChanged: {
            if (running) {
                root.state = "sending"
            }
        }
        onFinished: {
            if (error==0) {
                var resultUrl = output.url;
                console.log("Received", resultUrl, output.url)
                root.url = resultUrl;
                clipboard.content = resultUrl;

                root.pasteUrls.push(resultUrl);
                while (plasmoid.configuration.historySize <= root.pasteUrls.length && root.pasteUrls.length !== 0) {
                    root.pasteUrls.shift();
                }

                resetActions();
            }
            shareDialog.visible = false;
            root.state = error===0 ? "success" : "failure";
        }
    }

    function sendData(base64data, mimetype) {
//      Awesome KIO magic <3
        var url = "data:"+mimetype+";base64,"+base64data;

        shareDialog.inputData = {
            "urls": [url],
            "mimeType": mimetype
        }
        shareDialog.visible = true;
    }

    onDrop: {
        var mimetype = firstMimeType(event.mimeData.formats);
        var data = event.mimeData.getDataAsByteArray(mimetype);
        console.log("tosend", data)
        sendData(PurposeHelper.variantToBase64(data), mimetype);
        event.accepted = true;
    }

    property var pasteUrls: []
    property int nextPaste: 0

    Component.onCompleted: {
        plasmoid.setAction("paste", i18n("Paste"), "edit-paste");
        plasmoid.setActionSeparator("pastes");
    }

    function actionTriggered(actionName) {
        var index = parseInt(actionName);
        if (index)
            Qt.openUrlExternally(pasteUrls[actionName]);
    }

    function firstMimeType(formats) {
        for (var v in formats) {
            var curr = formats[v];
            console.log("trying...", curr, mimeDb.mimeTypeForName(curr) === {}, JSON.stringify(mimeDb.mimeTypeForName(curr)))
            if (mimeDb.mimeTypeForName(curr).hasOwnProperty("iconName"))
                return curr;
        }
        return "";
    }
    function action_paste() {
        var mimetype = firstMimeType(clipboard.formats);
        sendData(PurposeHelper.variantToBase64(clipboard.contentFormat(mimetype)), mimetype);
    }

    PlasmaCore.ToolTipArea {
        id: tooltipArea
        anchors.fill: parent
        location: plasmoid.location
        active: true
        mainText: i18n("Share")
        subText: i18n("Drop text or an image onto me to upload it to an online service.")
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
        radius: Math.max(parent.width, parent.height)/10
        anchors.fill: parent
        opacity: root.containsDrag || tooltipArea.containsMouse ? .3 : 0

        Behavior on opacity { NumberAnimation { duration: 100 } }
    }

    Timer {
        id: idleTimer
        running: false
        interval: 5000
        onTriggered: root.state = "idle"
    }

    state: "idle"
    states: [
        State {
            name: "idle"
            PropertyChanges { target: icon; source: "edit-paste" }
            PropertyChanges { target: tooltipArea; icon: "edit-paste" }
        },
        State {
            name: "sending"
            PropertyChanges { target: icon; visible: false }
            PropertyChanges { target: busy; visible: true }
            PropertyChanges { target: busy; running: true }

            PropertyChanges { target: tooltipArea; icon: "view-history" }
            PropertyChanges { target: tooltipArea; mainText: i18n("Sending...") }
            PropertyChanges { target: tooltipArea; subText: i18n("Please wait") }
        },
        State {
            name: "success"
            PropertyChanges { target: icon; source: "dialog-ok" }

            PropertyChanges { target: idleTimer; running: true }
            PropertyChanges { target: tooltipArea; icon: "dialog-ok" }
            PropertyChanges { target: tooltipArea; mainText: i18n("Successfully uploaded") }
            PropertyChanges { target: tooltipArea; subText: i18n("<a href='%1'>%1</a>", root.url) }
        },
        State {
            name: "failure"
            PropertyChanges { target: icon; source: "dialog-cancel" }

            PropertyChanges { target: idleTimer; running: true }
            PropertyChanges { target: tooltipArea; icon: icon.source }
            PropertyChanges { target: tooltipArea; mainText: i18n("Error during upload. Try again.") }
            PropertyChanges { target: tooltipArea; subText: i18n("Please, try again.") }
        }
    ]
}
