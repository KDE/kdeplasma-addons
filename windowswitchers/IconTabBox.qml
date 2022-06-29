/*
 KWin - the KDE window manager
 This file is part of the KDE project.

 SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

 SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import org.kde.plasma.core 2.0 as PlasmaCore

ListView {
    id: iconsListView

    property int iconSize

    focus: true
    orientation: ListView.Horizontal

    implicitWidth: contentWidth
    implicitHeight: iconSize + (highlightItem ? highlightItem.margins.top + highlightItem.margins.bottom : 0)

    delegate: PlasmaCore.IconItem {
        property string caption: model.caption

        width: iconSize + (highlightItem ? highlightItem.margins.left + highlightItem.margins.right : 0)
        height: iconSize + (highlightItem ? highlightItem.margins.top + highlightItem.margins.bottom : 0)

        source: model.icon
        active: index == iconsListView.currentIndex

        TapHandler {
            onSingleTapped: {
                if (index === ListView.view.currentIndex) {
                    ListView.view.model.activate(index);
                    return;
                }
                ListView.view.currentIndex = index;
            }
            onDoubleTapped: ListView.view.model.activate(index)
        }
    }

    highlight: PlasmaCore.FrameSvgItem {
        id: highlightItem
        imagePath: "widgets/viewitem"
        prefix: "hover"
        width: iconSize + margins.left + margins.right
        height: iconSize + margins.top + margins.bottom
    }

    highlightMoveDuration: 0
    highlightResizeDuration: 0
    boundsBehavior: Flickable.StopAtBounds
}
