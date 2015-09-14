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
import QtQuick.Layouts 1.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.draganddrop 2.0 as DragAndDrop
import org.kde.plasma.private.quicklaunch 1.0

import "../code/layout.js" as LayoutManager

Item {
    id: root

    property int maxSectionCount: Plasmoid.configuration.maxSectionCount
    property bool showLauncherNames : Plasmoid.configuration.showLauncherNames
    property bool vertical : Plasmoid.formFactor == PlasmaCore.Types.Vertical
    property bool horizontal : Plasmoid.formFactor == PlasmaCore.Types.Horizontal

    property bool dragging : false

    Layout.minimumWidth: LayoutManager.minimumWidth()
    Layout.minimumHeight: LayoutManager.minimumHeight()
    Layout.preferredWidth: LayoutManager.preferredWidth()
    Layout.preferredHeight: LayoutManager.preferredHeight()

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

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

            var index = grid.indexAt(event.x, event.y);

            if (isInternalDrop(event)) {
                launcherModel.moveUrl(event.mimeData.source.itemIndex, index);
            } else if (event.mimeData.hasUrls) {
                launcherModel.showDropMarker(index);
            }
        }

        onDragLeave: {
            dragging = false;
            launcherModel.clearDropMarker();
        }

        onDrop: {
            dragging = false;
            launcherModel.clearDropMarker();

            if (isInternalDrop(event)) {
                event.accept(Qt.IgnoreAction);
                saveConfiguration();
            } else if (event.mimeData.hasUrls) {
                var index = grid.indexAt(event.x, event.y);
                launcherModel.insertUrls(index == -1 ? grid.count : index, event.mimeData.urls);
                event.accept(event.proposedAction);
            }
        }
    }

    GridView {
        id: grid
        anchors.fill: parent
        interactive: false
        flow: horizontal ? GridView.FlowTopToBottom : GridView.FlowLeftToRight
        cellWidth: LayoutManager.preferredCellWidth()
        cellHeight: LayoutManager.preferredCellHeight()
        visible: count

        model: UrlModel {
            id: launcherModel
        }

        delegate: IconItem { }
    }

    PlasmaCore.FrameSvgItem {
        anchors.centerIn: parent
        width: Math.min(parent.height, parent.width)
        height: width
        imagePath: "widgets/viewitem"
        prefix: "hover"
        visible: dragging && defaultIcon.visible
    }

    PlasmaCore.IconItem {
        id: defaultIcon
        anchors.fill: parent
        source: "fork"
        visible: !grid.visible

        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            icon: "fork"
            mainText: i18n("Quicklaunch")
            subText: i18n("Add launchers by Drag and Drop or by using the context menu.")
        }
    }

    Logic {
        id: logic

        onLauncherAdded: {
            launcherModel.appendUrl(url);
        }

        onLauncherEdited: {
            launcherModel.changeUrl(index, url);
        }
    }

    Component.onCompleted: {
        launcherModel.setUrls(plasmoid.configuration.launcherUrls);
        launcherModel.urlsChanged.connect(saveConfiguration);

        plasmoid.setAction("addLauncher", i18n("Add Launcher..."), "list-add");
    }

    function action_addLauncher()
    {
        logic.addLauncher();
    }

    function saveConfiguration()
    {
        if (!dragging) {
            plasmoid.configuration.launcherUrls = launcherModel.urls();
        }
    }

    function isInternalDrop(event)
    {
        return event.mimeData.source
            && event.mimeData.source.GridView
            && event.mimeData.source.GridView.view == grid;
    }
}
