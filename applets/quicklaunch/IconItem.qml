/*
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.plasmoid
import org.kde.plasma.extras as PlasmaExtras
import plasma.applet.org.kde.plasma.quicklaunch

import "layout.js" as LayoutManager

Item {
    id: iconItem

    required property url url
    required property int index
    required property Logic logic
    required property GridView grid
    required property UrlModel popupModel
    required property UrlModel launcherModel

    readonly property int itemIndex : index
    property bool dragging : Drag.active
    property bool isPopupItem : false
    readonly property var launcher : logic.launcherData(url)
    readonly property string iconName : launcher.iconName || "fork"

    width: isPopupItem ? LayoutManager.popupItemWidth() : grid.cellWidth
    height: isPopupItem ? LayoutManager.popupItemHeight() : grid.cellHeight

    signal moveItemToGrid(IconItem item, url url)
    signal moveItemToPopup(IconItem item, url url)

    Keys.onPressed: event => {
        switch (event.key) {
        case Qt.Key_Space:
        case Qt.Key_Enter:
        case Qt.Key_Return:
        case Qt.Key_Select:
            logic.openUrl(url);
            break;
        case Qt.Key_Menu:
            contextMenu.open(0,0);
            event.accepted = true;
            break;
        case Qt.Key_Backspace:
        case Qt.Key_Delete:
            removeLauncher();
            event.accepted = true;
            break;
        }

        // BEGIN Arrow keys
        if (!(event.modifiers & Qt.ControlModifier) || !(event.modifiers & Qt.ShiftModifier)) {
            return;
        }

        switch (event.key) {
        case Qt.Key_Up: {
            if (iconItem.isPopupItem && iconItem.itemIndex === 0 && Plasmoid.location === PlasmaCore.Types.TopEdge) {
                iconItem.moveItemToGrid(iconItem, url);
                break;
            } else if (!iconItem.isPopupItem && Plasmoid.location === PlasmaCore.Types.BottomEdge) {
                iconItem.moveItemToPopup(iconItem, url);
                break;
            }

            decreaseIndex();
            break;
        }

        case Qt.Key_Down: {
            if (iconItem.isPopupItem && iconItem.itemIndex === iconItem.ListView.view.count - 1 && Plasmoid.location === PlasmaCore.Types.BottomEdge) {
                iconItem.moveItemToGrid(iconItem, url);
                break;
            } else if (!iconItem.isPopupItem && Plasmoid.location === PlasmaCore.Types.TopEdge) {
                iconItem.moveItemToPopup(iconItem, url);
                break;
            }

            increaseIndex();
            break;
        }

        case Qt.Key_Left: {
            if (iconItem.isPopupItem && Plasmoid.location === PlasmaCore.Types.LeftEdge) {
                iconItem.moveItemToGrid(iconItem, url);
                break;
            } else if (!iconItem.isPopupItem && Plasmoid.location === PlasmaCore.Types.RightEdge) {
                iconItem.moveItemToPopup(iconItem, url);
                break;
            }

            decreaseIndex();
            break;
        }
        case Qt.Key_Right: {
            if (iconItem.isPopupItem && Plasmoid.location === PlasmaCore.Types.RightEdge) {
                iconItem.moveItemToGrid(iconItem, url);
                break;
            } else if (!iconItem.isPopupItem && Plasmoid.location === PlasmaCore.Types.LeftEdge) {
                iconItem.moveItemToPopup(iconItem, url);
                break;
            }

            increaseIndex();
            break;
        }
        default:
            return;
        }

        event.accepted = true;
        // END Arrow keys
    }

    function decreaseIndex() {
        const newIndex = iconItem.itemIndex - 1;
        if (newIndex < 0) {
            return;
        }
        if (iconItem.isPopupItem) {
            popupModel.moveUrl(iconItem.itemIndex, newIndex);
            iconItem.ListView.view.currentIndex = newIndex;
        } else {
            launcherModel.moveUrl(iconItem.itemIndex, newIndex);
            iconItem.GridView.view.currentIndex = newIndex;
        }
    }

    function increaseIndex() {
        const newIndex = iconItem.itemIndex + 1;
        if (newIndex === (iconItem.isPopupItem ? iconItem.ListView.view.count : iconItem.GridView.view.count)) {
            return;
        }
        if (iconItem.isPopupItem) {
            popupModel.moveUrl(iconItem.itemIndex, newIndex);
            iconItem.ListView.view.currentIndex = newIndex;
        } else {
            launcherModel.moveUrl(iconItem.itemIndex, newIndex);
            iconItem.GridView.view.currentIndex = newIndex;
        }
    }

    DragHandler {
        id: dragHandler
        enabled: !Plasmoid.immutable
        target: null
        onActiveChanged: if (active) {
            icon.grabToImage(function(result) {
                iconItem.Drag.imageSource = result.url
                iconItem.Drag.active = true
            })
        } else {
            iconItem.Drag.active = false
        }
    }

    Drag.dragType: Drag.Automatic
    Drag.mimeData: { "text/uri-list": [iconItem.url] }
    Drag.proposedAction: Qt.MoveAction
    Drag.supportedActions: Qt.IgnoreAction | Qt.MoveAction

    Item {
        id: dragArea
        width: Math.min(iconItem.width, iconItem.height)
        height: width

        activeFocusOnTab: true
        Accessible.name: iconItem.launcher.applicationName
        Accessible.description: i18nc("@action:button accessible description", "Launch %1", iconItem.launcher.genericName || iconItem.launcher.applicationName)
        Accessible.role: Accessible.Button

        onActiveFocusChanged: {
            if (activeFocus && iconItem.ListView.view) {
                iconItem.ListView.view.currentIndex = iconItem.itemIndex;
            }
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: iconItem.logic.openUrl(iconItem.url)
        }

        TapHandler {
            acceptedButtons: Qt.RightButton
            onPressedChanged: if (pressed) {
                // needs callLater or the menu closes immediately
                Qt.callLater(() => contextMenu.open(point.pressPosition.x, point.pressPosition.y))
            }
        }

        HoverHandler {
            id: hoverHandler
            onHoveredChanged: if (hovered) {
                if (iconItem.ListView.view) {
                    iconItem.ListView.view.currentIndex = iconItem.itemIndex;
                }
            }
        }

        Kirigami.Icon {
            id: icon

            anchors {
                top: parent.top
                left: parent.left
            }

            width: Kirigami.Units.iconSizes.medium
            height: width
            source: iconItem.url === "quicklaunch:drop" ? "" : iconItem.iconName
            active: hoverHandler.hovered
        }

        PlasmaComponents3.Label {
            id: label

            anchors {
                bottom : parent.bottom
                right : parent.right
            }

            text: iconItem.launcher.applicationName
            textFormat: Text.PlainText
            maximumLineCount: 1
            wrapMode: Text.Wrap
        }

        KSvg.FrameSvgItem {
            anchors.fill: parent
            imagePath: "widgets/viewitem"
            prefix: "hover"
            visible: iconItem.dragging || iconItem.url === "quicklaunch:drop"
        }

        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            active: !iconItem.dragging
            mainText: iconItem.launcher.applicationName
            subText: iconItem.launcher.genericName
            icon: iconItem.iconName
        }

        PlasmaExtras.Menu {
            id: contextMenu

            property var jumpListItems : []

            visualParent: dragArea

            property Instantiator _children: Instantiator {
                id: instantiator

                model: iconItem.launcher.jumpListActions
                delegate: PlasmaExtras.MenuItem {
                    required icon
                    required property string name
                    required property string exec
                    text: name

                    onClicked: {
                        iconItem.logic.openExec(exec)
                    }
                }

                onObjectAdded: (index, object) => contextMenu.addMenuItem(object, jumpListSeparator)
                onObjectRemoved: (index, object) => contextMenu.removeMenuItem(object)
            }

            PlasmaExtras.MenuItem {
                id: jumpListSeparator
                separator: true
            }

            PlasmaExtras.MenuItem {
                text: i18nc("@action:inmenu", "Add Launcher…")
                icon: "list-add"
                onClicked: iconItem.addLauncher()
            }

            PlasmaExtras.MenuItem {
                text: i18nc("@action:inmenu", "Edit Launcher…")
                icon: "document-edit"
                onClicked: iconItem.editLauncher()
            }

            PlasmaExtras.MenuItem {
                text: i18nc("@action:inmenu", "Remove Launcher")
                icon: "list-remove"
                onClicked: iconItem.removeLauncher()
            }

            PlasmaExtras.MenuItem {
                separator: true
            }

            PlasmaExtras.MenuItem {
                action: Plasmoid.internalAction("configure")
            }

            PlasmaExtras.MenuItem {
                action: Plasmoid.internalAction("remove")
            }
        }
    }

    states: [
        State {
            name: "popup"
            when: iconItem.isPopupItem

            AnchorChanges {
                target: dragArea
                anchors.left: dragArea.parent.left
                anchors.right: dragArea.parent.right
                anchors.top: dragArea.parent.top
                anchors.bottom: dragArea.parent.bottom
            }

            AnchorChanges {
                target: icon
                anchors.verticalCenter: icon.parent.verticalCenter
                anchors.right: undefined
                anchors.bottom: undefined
            }

            AnchorChanges {
                target: label
                anchors.top: label.parent.top
                anchors.left: icon.right
            }

            PropertyChanges {
                icon.anchors.margins: LayoutManager.itemPadding()
                label.horizontalAlignment: Text.AlignLeft
                label.verticalAlignment: Text.AlignVCenter
                label.visible: true
                label.elide: Text.ElideRight
                label.anchors.leftMargin: Kirigami.Units.smallSpacing
                label.anchors.rightMargin: Kirigami.Units.smallSpacing
            }
        },

        State {
            name: "grid"
            when: !iconItem.isPopupItem

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
                label.horizontalAlignment: Text.AlignHCenter
                label.visible: showLauncherNames
                label.elide: Text.ElideNone
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
        const m = isPopupItem ? popupModel : launcherModel;
        m.removeUrl(itemIndex);
    }
}
