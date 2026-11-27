/*
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
pragma ComponentBehavior: Bound

import QtQuick

DropArea {
    id: root

    required property var model
    required property Item view

    property int internalDragIndex: -1
    property int internalDragOriginalIndex: -1

    signal saveConfigurationRequested()

    function indexAt(eventX, eventY) {
        const pos = view.mapFromItem(root, eventX, eventY);
        return (view as GridView)?.indexAt(pos.x, pos.y) ?? (view as ListView)?.indexAt(pos.x, pos.y);
    }

    onEntered: event => {
        if (event.hasUrls) {
            root.internalDragIndex = -1;
            root.internalDragOriginalIndex = -1;

            const urls = event.urls;
            if (urls.length === 1) {
                const dragUrl = urls[0].toString();
                const modelUrls = root.model.urls();
                for (let i = 0; i < modelUrls.length; ++i) {
                    if (modelUrls[i].toString() === dragUrl) {
                        root.internalDragIndex = i;
                        root.internalDragOriginalIndex = i;
                        break;
                    }
                }
            }
        } else {
            event.accepted = false;
        }
    }

    onPositionChanged: event => {
        const index = root.indexAt(event.x, event.y);

        if (root.internalDragIndex >= 0) {
            if (index >= 0 && index !== root.internalDragIndex) {
                root.model.move(root.internalDragIndex, index, 1);
                root.internalDragIndex = index;
            }
        } else {
            root.model.showDropMarker(index);
        }
    }

    onExited: {
        if (root.internalDragIndex >= 0) {
            if (root.internalDragIndex !== root.internalDragOriginalIndex) {
                root.model.move(root.internalDragIndex, root.internalDragOriginalIndex, 1);
            }
            root.internalDragIndex = -1;
            root.internalDragOriginalIndex = -1;
        } else {
            root.model.clearDropMarker();
        }
    }

    onDropped: event => {
        if (root.internalDragIndex >= 0) {
            root.internalDragIndex = -1;
            root.internalDragOriginalIndex = -1;
            event.accept(Qt.IgnoreAction);
            root.saveConfigurationRequested();
        } else {
            const index = root.indexAt(event.x, event.y);
            root.model.clearDropMarker();
            const urls = [...event.urls];
            event.accept(event.proposedAction);
            Qt.callLater(function() {
                root.model.insertUrls(index == -1 ? root.model.count : index, urls);
            });
        }
    }
}
