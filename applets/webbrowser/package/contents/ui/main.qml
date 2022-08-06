/*
 *   SPDX-FileCopyrightText: 2014, 2016 Mikhail Ivchenko <ematirov@gmail.com>
 *   SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@privat.broulik.de>
 *   SPDX-FileCopyrightText: 2020 Sora Steenvoort <sora@dillbox.me>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtWebEngine 1.5
import QtQuick.Layouts 1.1
import org.kde.plasma.components 2.0 as PlasmaComponents // for Menu+MenuItem
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

ColumnLayout {
    RowLayout{
        Layout.fillWidth: true
        PlasmaComponents3.Button {
            icon.name: "go-previous"
            onClicked: webview.goBack()
            enabled: webview.canGoBack
            display: PlasmaComponents3.AbstractButton.IconOnly
            text: i18nc("@action:button", "Go Back")
        }
        PlasmaComponents3.Button {
            icon.name: "go-next"
            onClicked: webview.goForward()
            enabled: webview.canGoForward
            display: PlasmaComponents3.AbstractButton.IconOnly
            text: i18nc("@action:button", "Go Forward")
        }
        PlasmaComponents3.TextField {
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

            Accessible.description: text.length > 0 ? text : i18nc("@info", "Type a URL")
        }

        // this shows page-related information such as blocked popups
        PlasmaComponents3.ToolButton {
            id: infoButton

            // callback invoked when button is clicked
            property var cb

            // button itself adds sufficient visual padding
            Layout.leftMargin: -parent.spacing
            Layout.rightMargin: -parent.spacing

            onClicked: cb();

            PlasmaComponents3.ToolTip {
                id: tooltip
            }

            function show(text, icon, tooltipText, cb) {
                infoButton.text = text;
                infoButton.icon.name = icon;
                tooltip.text = tooltipText;
                infoButton.cb = cb;
                infoButton.visible = true;
            }

            function dismiss() {
                infoButton.visible = false;
            }
        }

        PlasmaComponents3.Button {
            display: PlasmaComponents3.AbstractButton.IconOnly
            icon.name: webview.loading ? "process-stop" : "view-refresh"
            text: webview.loading ? i18nc("@action:button", "Stop Loading This Page") : i18nc("@action:button", "Reload This Page")
            onClicked: webview.loading ? webview.stop() : webview.reload()
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

        // TODO use contentsSize but that crashes, now mostly for some sane initial size
        Layout.preferredWidth: PlasmaCore.Units.gridUnit * 36
        Layout.preferredHeight: PlasmaCore.Units.gridUnit * 18

        // Binding it to e.g. width will be super slow on resizing
        Timer {
            id: updateZoomTimer
            interval: 100

            readonly property int minViewWidth: plasmoid.configuration.minViewWidth
            readonly property bool useMinViewWidth: plasmoid.configuration.useMinViewWidth
            readonly property int constantZoomFactor: plasmoid.configuration.constantZoomFactor

            onTriggered: {
                var newZoom = 1;
                if (useMinViewWidth) {
                    // Try to fit contents for a smaller screen
                    newZoom = Math.min(1, webview.width / minViewWidth);
                    // make sure value is valid
                    newZoom = Math.max(0.25, newZoom);
                } else {
                    newZoom = constantZoomFactor / 100.0;
                }
                webview.zoomFactor = newZoom;
                // setting the zoom factor does not always work on the first try; also, numbers get rounded
                if (Math.round(1000 * webview.zoomFactor) != Math.round(1000 * newZoom)) {
                    updateZoomTimer.restart();
                }
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

            readonly property bool useMinViewWidth : plasmoid.configuration.useMinViewWidth

            Connections {
                target: plasmoid.configuration
                
                function onMinViewWidthChanged() {updateZoomTimer.start()}

                function onUseMinViewWidthChanged() {updateZoomTimer.start()}

                function onConstantZoomFactorChanged() {updateZoomTimer.start()}

                function onUseConstantZoomChanged() {updateZoomTimer.start()}
            }

            onLinkHovered: {
                if (hoveredUrl.toString() !== "") {
                    mouseArea.cursorShape = Qt.PointingHandCursor;
                } else {
                    mouseArea.cursorShape = Qt.ArrowCursor;
                }
            }

            onWidthChanged: {
                if (useMinViewWidth) {
                    updateZoomTimer.start()
                }
            }

            onLoadingChanged: {
                if (loadRequest.status === WebEngineLoadRequest.LoadStartedStatus) {
                    infoButton.dismiss();
                } else if (loadRequest.status === WebEngineLoadRequest.LoadSucceededStatus && useMinViewWidth) {
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
