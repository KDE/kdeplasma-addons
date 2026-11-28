/*
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
pragma ComponentBehavior: Bound

import QtQuick
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.plasmoid
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.private.kicker as Kicker

PlasmaComponents3.ItemDelegate {
    id: iconItem

    required property string favoriteId
    required property int index
    required property var model

    required property Kicker.SimpleFavoritesModel favoritesModel

    readonly property int itemIndex : index
    property bool isPopupItem : false

    signal launcherDialogRequested()
    signal moveToOtherModelRequested(string favoriteId)

    activeFocusOnTab: true

    Drag.dragType: Drag.Automatic
    Drag.mimeData: { "text/uri-list": [iconItem.model.url] }
    Drag.proposedAction: Qt.MoveAction
    Drag.supportedActions: Qt.IgnoreAction | Qt.MoveAction

    Accessible.description: i18nc("@action:button accessible description", "Launch %1", iconItem.model.display || iconItem.model.description)

    onActiveFocusChanged: {
        if (activeFocus && iconItem.ListView.view) {
            iconItem.ListView.view.currentIndex = iconItem.itemIndex;
        }
    }

    onHoveredChanged: if (hovered && iconItem.isPopupItem) { iconItem.ListView.view.currentIndex = iconItem.itemIndex;}


    Keys.onPressed: event => {
        switch (event.key) {
        case Qt.Key_Enter:
        case Qt.Key_Return:
        case Qt.Key_Select:
            action.trigger();
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
                iconItem.moveToOtherModelRequested(iconItem.favoriteId);
                break;
            } else if (!iconItem.isPopupItem && Plasmoid.location === PlasmaCore.Types.BottomEdge) {
                iconItem.moveToOtherModelRequested(iconItem.favoriteId);
                break;
            }

            decreaseIndex();
            break;
        }

        case Qt.Key_Down: {
            if (iconItem.isPopupItem && iconItem.itemIndex === iconItem.ListView.view.count - 1 && Plasmoid.location === PlasmaCore.Types.BottomEdge) {
                iconItem.moveToOtherModelRequested(iconItem.favoriteId);
                break;
            } else if (!iconItem.isPopupItem && Plasmoid.location === PlasmaCore.Types.TopEdge) {
                iconItem.moveToOtherModelRequested(iconItem.favoriteId);
                break;
            }

            increaseIndex();
            break;
        }

        case Qt.Key_Left: {
            if (iconItem.isPopupItem && Plasmoid.location === PlasmaCore.Types.LeftEdge) {
                iconItem.moveToOtherModelRequested(iconItem.favoriteId);
                break;
            } else if (!iconItem.isPopupItem && Plasmoid.location === PlasmaCore.Types.RightEdge) {
                iconItem.moveToOtherModelRequested(iconItem.favoriteId);
                break;
            }

            decreaseIndex();
            break;
        }
        case Qt.Key_Right: {
            if (iconItem.isPopupItem && Plasmoid.location === PlasmaCore.Types.RightEdge) {
                iconItem.moveToOtherModelRequested(iconItem.favoriteId);
                break;
            } else if (!iconItem.isPopupItem && Plasmoid.location === PlasmaCore.Types.LeftEdge) {
                iconItem.moveToOtherModelRequested(iconItem.favoriteId);
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
        favoritesModel.moveRow(iconItem.itemIndex, newIndex)
        if (iconItem.isPopupItem) {
            iconItem.ListView.view.currentIndex = newIndex;
        } else {
            iconItem.GridView.view.currentIndex = newIndex;
        }
    }

    function increaseIndex() {
        const newIndex = iconItem.itemIndex + 1;
        if (newIndex === model.count) {
            return;
        }
        favoritesModel.moveRow(iconItem.itemIndex, newIndex)
        if (iconItem.isPopupItem) {
            iconItem.ListView.view.currentIndex = newIndex;
        } else {
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



    action: Kirigami.Action {
        onTriggered: {
            if (!dragHandler.active) {
                iconItem.favoritesModel.trigger(iconItem.index, "", null)
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

    icon.name: model.decoration || "fork"
    text: model.display

    PlasmaCore.ToolTipArea {
        anchors.fill: parent
        active: !iconItem.Drag.active
        mainText: iconItem.model.display ?? ""
        subText: iconItem.model.description ?? ""
        icon: iconItem.icon.name
    }

    PlasmaExtras.Menu {
        id: contextMenu

        visualParent: iconItem

        property Instantiator _children: Instantiator {
            id: instantiator

            model: iconItem.model.actionList
            delegate: PlasmaExtras.MenuItem {
                required property var model // can't be more specific, model may not provide them

                text      : model.text ?? ""
                enabled   : model.type !== "title" && (model.enabled ?? true)
                separator : model.type === "separator"
                section   : model.type === "title"
                icon      : model.icon ?? null
                checkable : model.checkable ?? false
                checked   : model.checked ?? false

                onClicked: {
                    iconItem.favoritesModel.trigger(iconItem.index, model.actionId, model.actionArgument);
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
        launcherDialogRequested();
    }

    function removeLauncher()
    {
        favoritesModel.removeFavorite(model.favoriteId);
    }
}
