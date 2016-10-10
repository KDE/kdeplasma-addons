/*
 *  Copyright 2015 David Rosca <nowrep@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.2
import org.kde.draganddrop 2.0 as DragAndDrop

import "../code/layout.js" as LayoutManager

Item {
    id: popup

    property bool dragging: false
    property alias popupModel : popupModel

    width: LayoutManager.popupItemWidth()
    height: Math.max(1, popupModel.count) * LayoutManager.popupItemHeight()

    DragAndDrop.DropArea {
        anchors.fill: parent
        preventStealing: true
        enabled: !plasmoid.immutable

        onDragEnter: {
            dragging = true;
        }

        onDragMove: {
            if (!event.mimeData.hasUrls) {
                return;
            }

            var index = listView.indexAt(event.x, event.y);

            if (isInternalDrop(event)) {
                popupModel.moveUrl(event.mimeData.source.itemIndex, index);
            } else if (event.mimeData.hasUrls) {
                popupModel.showDropMarker(index);
            }
        }

        onDragLeave: {
            dragging = false;
            popupModel.clearDropMarker();
        }

        onDrop: {
            dragging = false;
            popupModel.clearDropMarker();

            if (isInternalDrop(event)) {
                event.accept(Qt.IgnoreAction);
                saveConfiguration();
            } else if (event.mimeData.hasUrls) {
                var index = listView.indexAt(event.x, event.y);
                popupModel.insertUrls(index == -1 ? popupModel.count : index, event.mimeData.urls);
                event.accept(event.proposedAction);
            }
        }
    }

    ListView {
        id: listView
        anchors.fill: parent
        interactive: false

        model: UrlModel {
            id: popupModel
        }

        delegate: IconItem {
            isPopupItem: true
        }
    }

    Connections {
        target: plasmoid.configuration
        onPopupUrlsChanged: {
            popupModel.urlsChanged.disconnect(saveConfiguration);
            popupModel.setUrls(plasmoid.configuration.popupUrls);
            popupModel.urlsChanged.connect(saveConfiguration);
        }
    }

    Component.onCompleted: {
        popupModel.setUrls(plasmoid.configuration.popupUrls);
        popupModel.urlsChanged.connect(saveConfiguration);
    }

    function saveConfiguration()
    {
        if (!dragging) {
            plasmoid.configuration.popupUrls = popupModel.urls();
        }
    }

    function isInternalDrop(event)
    {
        return event.mimeData.source
            && event.mimeData.source.ListView
            && event.mimeData.source.ListView.view == listView;
    }
}
