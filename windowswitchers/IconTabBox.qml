/*
 KWin - the KDE window manager
 This file is part of the KDE project.

 SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

 SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: iconsTabBox
    function decrementCurrentIndex() {
        iconsListView.decrementCurrentIndex();
    }
    function incrementCurrentIndex() {
        iconsListView.incrementCurrentIndex();
    }
    property int iconSize
    property alias count: iconsListView.count
    property alias margins: hoverItem.margins
    property alias currentItem: iconsListView.currentItem
    property alias model: iconsListView.model
    property alias currentIndex: iconsListView.currentIndex
    focus: true
    clip: true

    // just to get the margin sizes
    PlasmaCore.FrameSvgItem {
        id: hoverItem
        imagePath: "widgets/viewitem"
        prefix: "hover"
        visible: false
    }

    // delegate
    Component {
        id: listDelegate
        Item {
            property alias caption: iconItem.caption
            id: delegateItem
            width: iconSize + hoverItem.margins.left + hoverItem.margins.right
            height: iconSize + hoverItem.margins.top + hoverItem.margins.bottom
            QIconItem {
                property variant caption: model.caption
                id: iconItem
                icon: model.icon
                width: iconSize
                height: iconSize
                state: index == iconsListView.currentIndex ? QIconItem.ActiveState : QIconItem.DefaultState
                anchors {
                    fill: parent
                    leftMargin: hoverItem.margins.left
                    rightMargin: hoverItem.margins.right
                    topMargin: hoverItem.margins.top
                    bottomMargin: hoverItem.margins.bottom
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    iconsListView.currentIndex = index;
                }
            }
        }
    }
    ListView {
        id: iconsListView
        orientation: ListView.Horizontal
        width: Math.min(parent.width, (iconSize + margins.left + margins.right) * count)
        height: iconSize + margins.top + margins.bottom
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
        clip: true
        delegate: listDelegate
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
}
