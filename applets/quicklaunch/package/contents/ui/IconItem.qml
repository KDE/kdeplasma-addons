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

import "layout.js" as LayoutManager

Item {
    id: iconItem

    readonly property int itemIndex : index
    property bool dragging : false
    property bool isPopupItem : false
    readonly property var launcher : logic.launcherData(url)
    readonly property string iconName : launcher.iconName || "fork"

    width: isPopupItem ? LayoutManager.popupItemWidth() : grid.cellWidth
    height: isPopupItem ? LayoutManager.popupItemHeight() : grid.cellHeight

    DragAndDrop.DragArea {
        id: dragArea
        width: Math.min(iconItem.width, iconItem.height)
        height: width
        enabled: !plasmoid.immutable
        defaultAction: Qt.MoveAction
        supportedActions: Qt.IgnoreAction | Qt.MoveAction
        delegate: icon

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
                removeLauncher();
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            anchors.margins: LayoutManager.itemPadding()
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onPressed: {
                if (mouse.button == Qt.RightButton) {
                    contextMenu.refreshActions();
                    contextMenu.open(mouse.x, mouse.y);
                }
            }

            onClicked: {
                if (mouse.button == Qt.LeftButton) {
                    logic.openUrl(url)
                }
            }

            PlasmaCore.IconItem {
                id: icon

                anchors {
                    top: parent.top
                    left: parent.left
                }

                width: units.iconSizes.medium
                height: width
                usesPlasmaTheme: false
                source: url == "quicklaunch:drop" ? "" : iconName
                active: mouseArea.containsMouse
            }

            PlasmaComponents.Label {
                id: label

                anchors {
                    bottom : parent.bottom
                    right : parent.right
                }

                text: iconItem.launcher.applicationName
                maximumLineCount: 1
                wrapMode: Text.Wrap
            }

            PlasmaCore.FrameSvgItem {
                anchors.fill: parent
                imagePath: "widgets/viewitem"
                prefix: "hover"
                visible: dragging || url == "quicklaunch:drop"
            }

            PlasmaCore.ToolTipArea {
                anchors.fill: parent
                active: !dragging
                mainText: iconItem.launcher.applicationName
                subText: iconItem.launcher.genericName
                icon: iconName
            }

            PlasmaComponents.ContextMenu {
                id: contextMenu

                property var jumpListItems : []

                visualParent: mouseArea

                PlasmaComponents.MenuItem {
                    id: jumpListSeparator
                    separator: true
                }

                PlasmaComponents.MenuItem {
                    text: i18nc("@action:inmenu", "Add Launcher...")
                    icon: "list-add"
                    onClicked: addLauncher()
                }

                PlasmaComponents.MenuItem {
                    text: i18nc("@action:inmenu", "Edit Launcher...")
                    icon: "document-edit"
                    onClicked: editLauncher()
                }

                PlasmaComponents.MenuItem {
                    text: i18nc("@action:inmenu", "Remove Launcher")
                    icon: "list-remove"
                    onClicked: removeLauncher()
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

                function refreshActions() {
                    for (var i = 0; i < jumpListItems.length; ++i) {
                        var item = jumpListItems[i];
                        removeMenuItem(item);
                        item.destroy();
                    }
                    jumpListItems = [];

                    for (var i = 0; i < launcher.jumpListActions.length; ++i) {
                        var action = launcher.jumpListActions[i];
                        var item = menuItemComponent.createObject(iconItem, {
                            "text": action.name,
                            "icon": action.icon
                        });
                        item.clicked.connect(function() {
                            logic.openExec(this.exec);
                        }.bind(action));

                        addMenuItem(item, jumpListSeparator);
                        jumpListItems.push(item);
                    }
                }
            }

            Component {
                id: menuItemComponent
                PlasmaComponents.MenuItem { }
            }
        }
    }

    states: [
        State {
            name: "popup"
            when: isPopupItem

            AnchorChanges {
                target: dragArea
                anchors.left: dragArea.parent.left
                anchors.right: dragArea.parent.right
                anchors.top: dragArea.parent.top
                anchors.bottom: dragArea.parent.bottom
            }

            AnchorChanges {
                target: icon
                anchors.right: undefined
                anchors.bottom: undefined
            }

            AnchorChanges {
                target: label
                anchors.top: label.parent.top
                anchors.left: icon.right
            }

            PropertyChanges {
                target: label
                horizontalAlignment: Text.AlignHLeft
                visible: true
                elide: Text.ElideRight
                anchors.leftMargin: units.smallSpacing
                anchors.rightMargin: units.smallSpacing
            }
        },

        State {
            name: "grid"
            when: !isPopupItem

            AnchorChanges {
                target: dragArea
                anchors.verticalCenter: dragArea.parent.verticalCenter
                anchors.horizontalCenter: dragArea.parent.horizontalCenter
            }

            AnchorChanges {
                target: icon
                anchors.right: icon.parent.right
                anchors.bottom: label.visible ? label.top : icon.parent.bottom
            }

            AnchorChanges {
                target: label
                anchors.top: undefined
                anchors.left: label.parent.left
            }

            PropertyChanges {
                target: label
                horizontalAlignment: Text.AlignHCenter
                visible: showLauncherNames
                elide: Text.ElideNone
            }
        }
    ]

    function addLauncher()
    {
        logic.addLauncher(isPopupItem);
    }

    function editLauncher()
    {
        logic.editLauncher(url, itemIndex, isPopupItem);
    }

    function removeLauncher()
    {
        var m = isPopupItem ? popupModel : launcherModel;
        m.removeUrl(itemIndex);
    }
}
