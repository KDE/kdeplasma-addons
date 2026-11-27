/*
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.plasmoid
import org.kde.plasma.extras as PlasmaExtras
import plasma.applet.org.kde.plasma.quicklaunch

import "layout.js" as LayoutManager

PlasmaComponents3.ItemDelegate {
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

    activeFocusOnTab: true

    Keys.onPressed: event => {
        switch (event.key) {
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
            iconItem.grabToImage(function(result) {
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

    Accessible.description: i18nc("@action:button accessible description", "Launch %1", iconItem.launcher.genericName || iconItem.launcher.applicationName)

    onActiveFocusChanged: {
        if (activeFocus && iconItem.ListView.view) {
            iconItem.ListView.view.currentIndex = iconItem.itemIndex;
        }
    }

    onHoveredChanged: if (hovered && iconItem.isPopupItem) { iconItem.ListView.view.currentIndex = iconItem.itemIndex;}

    action: Kirigami.Action {
        onTriggered: {
            if (!dragHandler.active) {
                iconItem.logic.openUrl(iconItem.url)
            }
        }
    }

    TapHandler {
        acceptedButtons: Qt.RightButton
        onPressedChanged: if (pressed) {
            // needs callLater or the menu closes immediately
            Qt.callLater(() => contextMenu.open(point.pressPosition.x, point.pressPosition.y))
        }
    }

    icon.name: iconItem.url === "quicklaunch:drop" ? "" : iconItem.iconName
    text: iconItem.launcher.applicationName

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

        visualParent: iconItem

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
