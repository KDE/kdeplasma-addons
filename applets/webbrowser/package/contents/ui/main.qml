/***************************************************************************
 *   Copyright 2014, 2016 by Mikhail Ivchenko <ematirov@gmail.com>         *
 *   Copyright 2018 by Kai Uwe Broulik <kde@privat.broulik.de>             *
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

import QtQuick 2.0
import QtWebEngine 1.5
import QtQuick.Layouts 1.1
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

ColumnLayout {
    RowLayout{
        Layout.fillWidth: true
        PlasmaComponents.Button{
            iconSource: "go-previous"
            onClicked: webview.goBack()
            enabled: webview.canGoBack
        }
        PlasmaComponents.Button{
            iconSource: "go-next"
            onClicked: webview.goForward()
            enabled: webview.canGoForward
        }
        PlasmaComponents.TextField{
            Layout.fillWidth: true
            onAccepted: {
                var url = text;
                if (url.indexOf(":/") < 0) {
                    url = "http://" + url;
                }
                webview.url = url;
            }
            onActiveFocusChanged: {
                if (activeFocus) {
                    selectAll();
                }
            }

            text: webview.url
        }

        // this shows page-related information such as blocked popups
        PlasmaComponents.ToolButton {
            id: infoButton

            // callback invoked when button is clicked
            property var cb

            // button itself adds sufficient visual padding
            Layout.leftMargin: -parent.spacing
            Layout.rightMargin: -parent.spacing

            onClicked: cb();

            function show(text, icon, tooltip, cb) {
                infoButton.text = text;
                infoButton.iconName = icon;
                infoButton.tooltip = tooltip;
                infoButton.cb = cb;
                infoButton.visible = true;
            }

            function dismiss() {
                infoButton.visible = false;
            }
        }

        PlasmaComponents.Button{
            iconSource: webview.loading ? "process-stop" : "view-refresh"
            onClicked: webview.loading ? webview.stop() : webview.reload()
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

        // TODO use contentsSize but that crashes, now mostly for some sane initial size
        Layout.preferredWidth: units.gridUnit * 25
        Layout.preferredHeight: units.gridUnit * 12

        // Binding it to e.g. width will be super slow on resizing
        Timer {
            id: updateZoomTimer
            interval: 100
            onTriggered: {
                // Try to fit contents for a smaller screen
                webview.zoomFactor = Math.min(1, webview.width / 1000);
            }
        }

        // This reimplements WebEngineView context menu for links to add a "open externally" entry
        // since you cannot add custom items there yet
        // there's a FIXME comment about that in QQuickWebEngineViewPrivate::contextMenuRequested
        PlasmaComponents.Menu {
            id: linkContextMenu
            visualParent: webview

            property string link

            PlasmaComponents.MenuItem {
                text: i18nc("@action:inmenu", "Open Link in Browser")
                icon:  "internet-web-browser"
                onClicked: Qt.openUrlExternally(linkContextMenu.link)
            }

            PlasmaComponents.MenuItem {
                text: i18nc("@action:inmenu", "Copy Link Address")
                icon: "edit-copy"
                onClicked: webview.triggerWebAction(WebEngineView.CopyLinkToClipboard)
            }
        }

        WebEngineView {
            id: webview
            anchors.fill: parent
            onUrlChanged: plasmoid.configuration.url = url;
            Component.onCompleted: url = plasmoid.configuration.url;

            onLinkHovered: {
                if (hoveredUrl.toString() !== "") {
                    mouseArea.cursorShape = Qt.PointingHandCursor;
                } else {
                    mouseArea.cursorShape = Qt.ArrowCursor;
                }
            }

            onWidthChanged: updateZoomTimer.start()
            onLoadingChanged: {
                if (loadRequest.status === WebEngineLoadRequest.LoadStartedStatus) {
                    infoButton.dismiss();
                } else if (loadRequest.status === WebEngineLoadRequest.LoadSucceededStatus) {
                    updateZoomTimer.start();
                }
            }

            onContextMenuRequested: {
                if (request.mediaType === ContextMenuRequest.MediaTypeNone && request.linkUrl.toString() !== "") {
                    linkContextMenu.link = request.linkUrl;
                    linkContextMenu.open(request.x, request.y);
                    request.accepted = true;
                }
            }

            onNewViewRequested: {
                var url = request.requestedUrl;

                if (request.userInitiated) {
                    Qt.openUrlExternally(url);
                } else {
                    infoButton.show(i18nc("An unwanted popup was blocked", "Popup blocked"), "document-close",
                                    i18n("Click here to open the following blocked popup:\n%1", url), function () {
                        Qt.openUrlExternally(url);
                        infoButton.dismiss();
                    });
                }
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            acceptedButtons: Qt.BackButton | Qt.ForwardButton
            onPressed: {
                if (mouse.button === Qt.BackButton) {
                    webview.goBack();
                } else if (mouse.button === Qt.ForwardButton) {
                    webview.goForward();
                }
            }
        }
    }
}
