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

DropArea {
    id: root

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Layout.fillWidth: false
    Layout.fillHeight:false
    Layout.minimumWidth: 0
    Layout.minimumHeight: 0

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
            var source = plasmoid.configuration[category]

            if (source)
                return { "mime": mime, "format": mimeNames[i], "source": source };
        }
        return {}
    }

    onEntered: {
        var source = preferredSourceForMimetypes(drag.formats, [drag.getDataAsString("text/uri-list")]);

        icon.source = source.mime.iconName;
        drag.accepted = true
    }

    QtExtra.Clipboard {
        id: clipboard
    }

    QtExtra.MimeDatabase {
        id: mimeDb
    }

    function resetActions() {
        for(var v in root.pasteUrls) {
            plasmoid.setAction("showpaste"+(v+1), root.pasteUrls[v], "");
        }
    }

    function sendData(source, data) {
        var service = shareDataSource.serviceForSource(source);
        var operation = service.operationDescription("share");
        operation.content = data;
        root.lastJob = service.startOperationCall(operation);
        root.lastJob.finished.connect(function(){
            var resultUrl = root.lastJob.result;

            if (root.lastJob.error==0) {
                root.url = resultUrl;
                clipboard.content = resultUrl;

                root.pasteUrls.push(resultUrl);
                if (plasmoid.configuration.historySize <= root.pasteUrls.length) {
                    root.pasteUrls.shift();
                }

                resetActions();
            }
            root.state = root.lastJob.error==0 ? "success" : "failure";
        });
        root.state = "sending";
    }

    onDropped: {
        var pref = preferredSourceForMimetypes(drop.formats);
        sendData(pref.source, drop.getDataAsString(pref.format))
        drop.accepted = true;
    }

    onExited: icon.source = "edit-paste"

    property var pasteUrls: []
    property int nextPaste: 0

    Component.onCompleted: {
        plasmoid.setAction("paste", i18n("Paste"), "edit-paste");
        plasmoid.setActionSeparator("pastes");
    }

    //FIXME somehow we should get these to be generic, now the history doesn't go further than 5 :D
    function action_showpaste1() { Qt.openUrlExternally(pasteUrls[0]); }
    function action_showpaste2() { Qt.openUrlExternally(pasteUrls[1]); }
    function action_showpaste3() { Qt.openUrlExternally(pasteUrls[2]); }
    function action_showpaste4() { Qt.openUrlExternally(pasteUrls[3]); }
    function action_showpaste5() { Qt.openUrlExternally(pasteUrls[4]); }

    function action_paste() {
        var pref = preferredSourceForMimetypes(clipboard.formats, clipboard.contentFormat("text/uri-list"));

        sendData(pref.source, clipboard.contentFormat(pref.format));
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

    PlasmaCore.DataSource {
        id: shareDataSource
        engine: "org.kde.plasma.dataengine.share"
        connectedSources: sources
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
