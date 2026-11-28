/*
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid
import org.kde.plasma.private.kicker as Kicker

import plasma.applet.org.kde.plasma.quicklaunch
import "layout.js" as LayoutManager

Item {
    id: popup

    property alias model: listView.model
    property alias listView: listView
    required property Kicker.SimpleFavoritesModel favoritesModel

    signal launcherDialogRequested
    signal addToWidgetModelRequested(string id)

    implicitWidth: LayoutManager.popupItemWidth()
    implicitHeight: Math.max(1, listView.count) * LayoutManager.popupItemHeight()

    QuickLaunchDropArea {
        id: dropArea
        anchors.fill: parent
        enabled: !Plasmoid.immutable

        model: listView.model
        view: listView
    }

    ListView {
        id: listView
        anchors.fill: parent

        focus: true
        interactive: true
        keyNavigationWraps: true

        delegate: IconItem {
            width: LayoutManager.popupItemWidth()
            height: LayoutManager.popupItemHeight()

            favoritesModel: popup.favoritesModel
            isPopupItem: true
            onMoveToOtherModelRequested: id => {
                popup.addToWidgetModelRequested(id)
                popup.favoritesModel.removeFavorite(id)
                listView.currentIndex = popup.favoritesModel.count - 1
            }
            onLauncherDialogRequested: popup.launcherDialogRequested()
        }

        highlight: PlasmaExtras.Highlight {}

        highlightMoveDuration: 0
    }
}
