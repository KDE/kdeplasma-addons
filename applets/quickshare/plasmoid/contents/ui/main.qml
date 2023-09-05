/*
 *   SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0 as QtExtra
import QtQuick.Layouts 1.1
import org.kde.plasma.private.purpose 1.0

PlasmoidItem {
    id: root

    preferredRepresentation: fullRepresentation

    readonly property bool inPanel: (plasmoid.location === PlasmaCore.Types.TopEdge
        || plasmoid.location === PlasmaCore.Types.RightEdge
        || plasmoid.location === PlasmaCore.Types.BottomEdge
        || plasmoid.location === PlasmaCore.Types.LeftEdge)

    Layout.minimumWidth: Kirigami.Units.iconSizes.small
    Layout.minimumHeight: Layout.minimumWidth

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
            performPaste();
        }
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            text: i18nc("@action", "Paste")
            icon.name: "edit-paste"
            onTriggered: performPaste()
        },
        PlasmaCore.Action {
            isSeparator: true
        }
    ]

    Instantiator {
        model: root.pasteUrls
        delegate: PlasmaCore.Action {
            text: modelData
            onTriggered: Qt.openUrlExternally(modelData)
        }
        onObjectAdded: (index, object) => {
            Plasmoid.contextualActions.push(object)
        }
        onObjectRemoved: (index, object) => {
            Plasmoid.contextualActions.splice(contextualActions.indexOf(object), 1)
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

    function performPaste() {
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

    function copyUrl(url) {
        if (plasmoid.configuration.copyAutomatically)
            clipboard.content = url;
        else {
            showUrl.url = url
            showUrl.visible = true
        }
    }

    DropArea {
        anchors.fill: parent
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

        ContentTracker {
            id: contentTracker
        }

        MouseArea {
            anchors.fill: parent
            activeFocusOnTab: true
            Keys.onPressed: {
                switch (event.key) {
                case Qt.Key_Space:
                case Qt.Key_Enter:
                case Qt.Key_Return:
                case Qt.Key_Select:
                    activate();
                    break;
                }
            }
            Accessible.name: tooltipArea.mainText
            Accessible.description: tooltipArea.subText
            Accessible.role: Accessible.Button
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
            onFinished: (output, error, message) => {
                if (error == 0 && output.hasOwnProperty("url") && output.url !== "") {
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

                    root.pasteUrlsChanged();
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

        PlasmaCore.ToolTipArea {
            id: tooltipArea
            anchors.fill: parent
            location: plasmoid.location
            active: true
            mainText: i18n("Share")
            subText: i18n("Drop text or an image onto me to upload it to an online service.")
        }

        Kirigami.Icon {
            id: icon
            anchors.fill: parent
        }

        PlasmaComponents3.BusyIndicator {
            id: busy
            anchors.centerIn: parent
            visible: false
        }

        Rectangle {
            border {
                color: Kirigami.Theme.textColor
                width: root.containsDrag ? 5 : 1
            }
            color: Kirigami.Theme.backgroundColor
            radius: Math.max(parent.width, parent.height)/10
            anchors.fill: parent
            opacity: root.containsDrag || tooltipArea.containsMouse ? .3 : 0

            Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
        }

        Timer {
            id: idleTimer
            running: false
            interval: 5000
            onTriggered: root.state = "idle"
        }

    }
    state: "idle"
    states: [
        State {
            name: "idle"
            PropertyChanges { target: icon; source: "emblem-shared-symbolic" }
            PropertyChanges { target: tooltipArea; mainText: i18n("Share") }
            PropertyChanges { target: tooltipArea; subText: contentTracker.uri ? i18n("Upload %1 to an online service", contentTracker.title ? contentTracker.title : filenameFromUrl(contentTracker.uri)) :  i18n("Drop text or an image onto me to upload it to an online service.") }
        },
        State {
            name: "configuration"
            PropertyChanges { target: icon; source: "configure" }
            PropertyChanges { target: shareDialog; visible: true }
        },
        State {
            name: "sending"
            PropertyChanges { target: icon; visible: false }
            PropertyChanges { target: busy; visible: true }
            PropertyChanges { target: busy; running: true }

            PropertyChanges { target: tooltipArea; mainText: i18n("Sending…") }
            PropertyChanges { target: tooltipArea; subText: i18n("Please wait") }
        },
        State {
            name: "success"
            PropertyChanges { target: icon; source: "dialog-ok" }

            PropertyChanges { target: idleTimer; running: true }
            PropertyChanges { target: tooltipArea; mainText: i18n("Successfully uploaded") }
            PropertyChanges { target: tooltipArea; subText: root.url ? i18n("<a href='%1'>%1</a>", root.url) : "" }
        },
        State {
            name: "failure"
            PropertyChanges { target: icon; source: "dialog-cancel" }

            PropertyChanges { target: idleTimer; running: true }
            PropertyChanges { target: tooltipArea; mainText: i18n("Error during upload.") }
            PropertyChanges { target: tooltipArea; subText: (root.errorMessage=="" ? i18n("Please, try again.") : root.errorMessage) }
        }
    ]
}
