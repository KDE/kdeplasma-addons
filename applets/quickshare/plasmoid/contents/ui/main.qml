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

DropArea {
    id: root

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    readonly property bool inPanel: (plasmoid.location === PlasmaCore.Types.TopEdge
        || plasmoid.location === PlasmaCore.Types.RightEdge
        || plasmoid.location === PlasmaCore.Types.BottomEdge
        || plasmoid.location === PlasmaCore.Types.LeftEdge)

    Layout.minimumWidth: units.iconSizes.small
    Layout.minimumHeight: Layout.minimumWidth

    Layout.maximumWidth: inPanel ? units.iconSizeHints.panel : -1
    Layout.maximumHeight: inPanel ? units.iconSizeHints.panel : -1

    property string url: ""
    property string errorMessage: ""
    property var pasteUrls: []
    property int nextPaste: 0

    function firstMimeType(formats) {
        for (var v in formats) {
            var curr = formats[v];

            if (curr == "text/uri-list") {
                return { name: "text/uri-list" };
            } else {
                var mime = mimeDb.mimeTypeForName(curr);
                if (mime.hasOwnProperty("iconName")) {
                    return mime;
                }
            }
        }
        return {};
    }

    function urlsMimetype(urls)
    {
        return mimeDb.mimeTypeForUrl(urls[0]);
    }

    function filenameFromUrl(url) {
        var parts = url.split("/");
        return parts[parts.length - 1];
    }

    function objectToArray(object) {
        var array = [];
        for(var v in object) {
            // toString() here too because sometimes the contents are non-string (eg QUrl)
            array.push(object[v].toString());
        }
        return array;
    }

    onEntered: {
        root.state = "idle";
        var mimetype;
        if (drag.hasUrls) {
            mimetype = urlsMimetype(objectToArray(drag.urls));
        } else {
            mimetype = firstMimeType(drag.formats);
        }
        icon.source = mimetype.iconName;
        drag.accepted = true
    }
    onExited: {
        icon.source = "edit-paste"
    }

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

    ContentTracker {
        id: contentTracker
    }

    function activate() {
        if (root.state == "configuration") {
            root.state = "idle";
        } else if (contentTracker.uri) {
            var mime = contentTracker.mimeType;
            if (!mime) {
                mime = mimeDb.mimeTypeForUrl(contentTracker.uri).name;
            }
            sendData([contentTracker.uri], mime);
        } else {
            action_paste();
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            activate();
        }
    }
    Plasmoid.onActivated: {
        activate();
    }

    ShowUrlDialog {
        id: showUrl
        location: plasmoid.location
        visualParent: parent
        onCopyUrl: {
            clipboard.content = showUrl.url;
        }
    }

    function copyUrl(url) {
        if (plasmoid.configuration.copyAutomatically)
            clipboard.content = url;
        else {
            showUrl.url = url
            showUrl.visible = true
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
            if (error==0 && output.url !== "") {
                console.assert(output.url !== undefined);
                var resultUrl = output.url;
                console.log("Received url", resultUrl)
                if (resultUrl) {
                    root.url = resultUrl;
                    copyUrl(resultUrl)

                    root.pasteUrls.push(resultUrl);
                    while (plasmoid.configuration.historySize <= root.pasteUrls.length && root.pasteUrls.length !== 0) {
                        root.pasteUrls.shift();
                    }
                }

                resetActions();
            }
            shareDialog.visible = false;
            root.state = error===0 ? "success" : "failure";
            root.errorMessage = message;
        }
        onWindowDeactivated: {
            if (!running)
                root.state = "idle";
        }
    }

    function sendBase64Data(base64data, mimetype) {
//      Awesome KIO magic <3
        var url = "data:"+mimetype+";base64,"+base64data;
        sendData([url], mimetype);
    }

    function sendData(urls, mimetype) {
        shareDialog.inputData = {
            "urls": urls,
            "mimeType": mimetype
        }
        root.state = "configuration"
    }

    onDropped: {
        if (drop.hasUrls) {
            var urls = objectToArray(drop.urls);

            sendData(urls, urlsMimetype(urls).name);
        } else {
            var mimetype = firstMimeType(drop.formats).name;
            var data = drop.getDataAsArrayBuffer(mimetype);
            sendBase64Data(PurposeHelper.variantToBase64(data), mimetype);
        }
        drop.accepted = true;
    }

    Component.onCompleted: {
        plasmoid.setAction("paste", i18nc("@action", "Paste"), "edit-paste");
        plasmoid.setActionSeparator("pastes");
    }

    function actionTriggered(actionName) {
        var index = parseInt(actionName);
        if (index)
            Qt.openUrlExternally(pasteUrls[actionName]);
    }

    function action_paste() {
        if (clipboard.formats.length < 1) { // empty clipboard!
            return; // do nothing (there's the tooltip!)
        }
        if (clipboard.formats.indexOf("text/uri-list")>=0) {
            var urls = objectToArray(clipboard.contentFormat("text/uri-list"))
            sendData(urls, urlsMimetype(urls).name);
        } else {
            var mimetype = firstMimeType(clipboard.formats).name;
            sendBase64Data(PurposeHelper.variantToBase64(clipboard.contentFormat(mimetype)), mimetype);
        }
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
            PropertyChanges { target: icon; source: "emblem-shared-symbolic" }
            PropertyChanges { target: tooltipArea; icon: "emblem-shared-symbolic" }
            PropertyChanges { target: tooltipArea; mainText: i18n("Share") }
            PropertyChanges { target: tooltipArea; subText: contentTracker.uri ? i18n("Upload %1 to an online service", contentTracker.title ? contentTracker.title : filenameFromUrl(contentTracker.uri)) :  i18n("Drop text or an image onto me to upload it to an online service.") }
        },
        State {
            name: "configuration"
            PropertyChanges { target: icon; source: "configure" }
            PropertyChanges { target: tooltipArea; icon: "edit-paste" }
            PropertyChanges { target: shareDialog; visible: true }
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
            PropertyChanges { target: tooltipArea; subText: root.url ? i18n("<a href='%1'>%1</a>", root.url) : "" }
        },
        State {
            name: "failure"
            PropertyChanges { target: icon; source: "dialog-cancel" }

            PropertyChanges { target: idleTimer; running: true }
            PropertyChanges { target: tooltipArea; icon: icon.source }
            PropertyChanges { target: tooltipArea; mainText: i18n("Error during upload.") }
            PropertyChanges { target: tooltipArea; subText: (root.errorMessage=="" ? i18n("Please, try again.") : root.errorMessage) }
        }
    ]
}
