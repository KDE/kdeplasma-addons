/*
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
pragma ComponentBehavior: Bound

import QtQuick
import org.kde.plasma.private.kicker as Kicker

DropArea {
    id: root

    required property Kicker.SimpleFavoritesModel model
    required property Item view

    property int internalDragIndex: -1
    property int internalDragOriginalIndex: -1

    function itemAt(eventX, eventY) {
        const pos = view.mapFromItem(root, eventX, eventY);
        return (view as GridView)?.itemAt(pos.x, pos.y) ?? (view as ListView)?.itemAt(pos.x, pos.y);
    }

    onEntered: event => {
        const eventSource = event.source as IconItem
        if (eventSource && model.isFavorite(eventSource.model.favoriteId)) {
            root.internalDragIndex = eventSource.index;
            root.internalDragOriginalIndex = eventSource.index;
        } else {
            event.accepted = event.hasUrls;
        }
    }

    onPositionChanged: event => {
        const above = root.itemAt(event.x, event.y) as IconItem
        const eventSource = event.source as IconItem

        if (!above || above === eventSource) {
            return
        } else if (eventSource && model.isFavorite(eventSource.model.favoriteId)) {
            model.moveRow(eventSource.itemIndex, above.itemIndex);
            root.internalDragIndex = eventSource.index;
            event.accept(Qt.MoveAction)
        } else {
            model.dropPlaceholderIndex = above?.itemIndex ?? 0;
            event.accept(Qt.CopyAction)
        }
    }

    onExited: {
        model.dropPlaceholderIndex = -1
        if (root.internalDragIndex > -1) {
            model.moveRow(root.internalDragIndex, root.internalDragOriginalIndex)
        }
        // TODO: restore internal drag item at original position?
    }

    onDropped: event => {
        if (root.internalDragOriginalIndex >= 0) {
            root.internalDragOriginalIndex = -1;
            root.internalDragIndex = -1;
            event.accept(Qt.MoveAction);
        } else {
            const index = model.dropPlaceholderIndex;
            const urls = [...event.urls];
            event.accept(event.proposedAction);
            Qt.callLater(function() {
                for (let i = 0; i < urls.length; i++) {
                    model.addFavorite(urls[i], index + i)
                }
            });
        }
        model.dropPlaceholderIndex = -1;
    }
}
