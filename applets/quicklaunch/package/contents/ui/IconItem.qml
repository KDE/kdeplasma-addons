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
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.draganddrop 2.0 as DragAndDrop

import "../code/layout.js" as LayoutManager

Item {
    id: iconItem

    property int itemIndex : index
    property bool dragging : false
    property var launcher : logic.launcherData(url)

    width: grid.cellWidth
    height: grid.cellHeight

    DragAndDrop.DragArea {
        id: dragArea

        anchors {
            centerIn: parent
            margins: LayoutManager.itemPadding()
        }

        width: Math.min(iconItem.width, iconItem.height)
        height: width
        enabled: !plasmoid.immutable
        defaultAction: Qt.MoveAction
        supportedActions: Qt.IgnoreAction | Qt.MoveAction
        delegateImage: logic.iconFromTheme(launcher.iconName, Qt.size(icon.width, icon.height))

        mimeData {
            url: url
            source: iconItem
        }

        onDragStarted: {
            dragging = true;
        }

        onDrop: {
            dragging = false;

            if (action == Qt.MoveAction) {
                launcherModel.removeUrl(itemIndex);
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onClicked: {
                if (mouse.button == Qt.LeftButton) {
                    logic.openUrl(url)
                } else if (mouse.button == Qt.RightButton) {
                    contextMenu.open(mouse.x, mouse.y);
                }
            }

            PlasmaCore.IconItem {
                id: icon

                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    bottom: text.visible ? text.top : parent.bottom
                }

                source: iconItem.launcher.iconName;
                active: mouseArea.containsMouse
            }

            PlasmaComponents.Label {
                id: text

                anchors {
                    left : parent.left
                    bottom : parent.bottom
                    right : parent.right
                }

                text: iconItem.launcher.applicationName
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 1
                wrapMode: Text.Wrap
                visible: showLauncherNames
            }

            PlasmaCore.FrameSvgItem {
                anchors.fill: parent
                imagePath: "widgets/viewitem"
                prefix: "hover"
                visible: dragging || url == "quicklaunch:drop"
            }

            PlasmaCore.ToolTipArea {
                anchors.fill: parent
                mainText: iconItem.launcher.applicationName
                subText: iconItem.launcher.genericName
                icon: iconItem.launcher.iconName
            }

            PlasmaComponents.ContextMenu {
                id: contextMenu
                visualParent: mouseArea

                PlasmaComponents.MenuItem {
                    action: plasmoid.action("addLauncher")
                }

                PlasmaComponents.MenuItem {
                    text: i18n("Edit Launcher...")
                    icon: "document-edit"
                    onClicked: logic.editLauncher(url, itemIndex)
                }

                PlasmaComponents.MenuItem {
                    text: i18n("Remove Launcher")
                    icon: "list-remove"
                    onClicked: launcherModel.removeUrl(itemIndex)
                }

                PlasmaComponents.MenuItem {
                    separator: true
                }

                PlasmaComponents.MenuItem {
                    action: plasmoid.action("configure")
                }

                PlasmaComponents.MenuItem {
                    action: plasmoid.action("remove")
                }
            }
        }
    }
}
