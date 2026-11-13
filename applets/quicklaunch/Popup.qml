/*
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid

import org.kde.draganddrop as DragAndDrop

import plasma.applet.org.kde.plasma.quicklaunch
import "layout.js" as LayoutManager

Item {
    id: popup

    property bool dragging: false
    property int internalDragIndex: -1
    property int internalDragOriginalIndex: -1
    property alias popupModel : popupModelObject
    property alias listView: listView
    required property Logic logic
    required property UrlModel launcherModel

    width: LayoutManager.popupItemWidth()
    height: Math.max(1, popupModel.count) * LayoutManager.popupItemHeight()

    DragAndDrop.DropArea {
        anchors.fill: parent
        preventStealing: true
        enabled: !Plasmoid.immutable

        onDragEnter: event => {
            if (event.mimeData.hasUrls) {
                popup.dragging = true;
                popup.internalDragIndex = -1;
                popup.internalDragOriginalIndex = -1;

                const urls = event.mimeData.urls;
                if (urls.length === 1) {
                    const dragUrl = urls[0].toString();
                    const modelUrls = popupModelObject.urls();
                    for (let i = 0; i < modelUrls.length; ++i) {
                        if (modelUrls[i].toString() === dragUrl) {
                            popup.internalDragIndex = i;
                            popup.internalDragOriginalIndex = i;
                            break;
                        }
                    }
                }
            } else {
                event.ignore();
            }
        }

        onDragMove: event => {
            const index = listView.indexAt(event.x, event.y);

            if (popup.internalDragIndex >= 0) {
                if (index >= 0 && index !== popup.internalDragIndex) {
                    popupModelObject.move(popup.internalDragIndex, index, 1);
                    popup.internalDragIndex = index;
                }
            } else {
                popupModelObject.showDropMarker(index);
            }
        }

        onDragLeave: {
            popup.dragging = false;
            if (popup.internalDragIndex >= 0) {
                if (popup.internalDragIndex !== popup.internalDragOriginalIndex) {
                    popupModelObject.move(popup.internalDragIndex, popup.internalDragOriginalIndex, 1);
                }
                popup.internalDragIndex = -1;
                popup.internalDragOriginalIndex = -1;
            } else {
                popupModelObject.clearDropMarker();
            }
        }

        onDrop: event => {
            popup.dragging = false;

            if (popup.internalDragIndex >= 0) {
                popup.internalDragIndex = -1;
                popup.internalDragOriginalIndex = -1;
                event.accept(Qt.IgnoreAction);
                popup.saveConfiguration();
            } else if (event.mimeData.hasUrls) {
                const index = listView.indexAt(event.x, event.y);
                popupModelObject.clearDropMarker();
                const urls = event.mimeData.urls;
                event.accept(event.proposedAction);
                Qt.callLater(function() {
                    popupModelObject.insertUrls(index == -1 ? popupModelObject.count : index, urls);
                });
            }
        }
    }

    ListView {
        id: listView
        anchors.fill: parent

        focus: true
        interactive: true
        keyNavigationWraps: true

        model: UrlModel {
            id: popupModelObject
        }

        delegate: IconItem {
            isPopupItem: true
            popupModel: popupModelObject
            launcherModel: popup.launcherModel
            grid: null
            logic: popup.logic
            onMoveItemToGrid: (item, url) => listView.moveItemToGrid(item, url)
        }

        highlight: PlasmaExtras.Highlight {}

        highlightMoveDuration: Kirigami.Units.longDuration
        highlightMoveVelocity: 1

        function moveItemToGrid(iconItem, url) {
            popup.launcherModel.insertUrl(popup.launcherModel.count, url.toString());
            listView.currentIndex = popup.launcherModel.count - 1;
            iconItem.removeLauncher();
        }
    }

    Connections {
        target: Plasmoid.configuration
        function onPopupUrlsChanged() {
            if (popup.dragging) return;
            const configUrls = Plasmoid.configuration.popupUrls;
            const modelUrls = popupModelObject.urls();
            if (configUrls.length === modelUrls.length) {
                let same = true;
                for (let i = 0; i < configUrls.length; ++i) {
                    if (configUrls[i].toString() !== modelUrls[i].toString()) {
                        same = false;
                        break;
                    }
                }
                if (same) return;
            }
            popupModelObject.urlsChanged.disconnect(popup.saveConfiguration);
            popupModelObject.setUrls(Plasmoid.configuration.popupUrls);
            popupModelObject.urlsChanged.connect(popup.saveConfiguration);
        }
    }

    Component.onCompleted: {
        popupModelObject.setUrls(Plasmoid.configuration.popupUrls);
        popupModelObject.urlsChanged.connect(saveConfiguration);
    }

    function saveConfiguration()
    {
        if (!dragging) {
            Plasmoid.configuration.popupUrls = popupModelObject.urls();
        }
    }
}
