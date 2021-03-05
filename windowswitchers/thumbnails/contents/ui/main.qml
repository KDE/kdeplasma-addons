/*
 KWin - the KDE window manager
 This file is part of the KDE project.

 SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

 SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kwin 2.0 as KWin

KWin.Switcher {
    id: tabBox
    currentIndex: thumbnailListView.currentIndex

    PlasmaCore.Dialog {
        id: dialog
        location: PlasmaCore.Types.Floating
        visible: tabBox.visible
        flags: Qt.X11BypassWindowManagerHint
        x: tabBox.screenGeometry.x + tabBox.screenGeometry.width * 0.5 - dialogMainItem.width * 0.5
        y: tabBox.screenGeometry.y + tabBox.screenGeometry.height * 0.5 - dialogMainItem.height * 0.5
        mainItem: Item {
            id: dialogMainItem
            property real screenFactor: tabBox.screenGeometry.width/tabBox.screenGeometry.height
            property int optimalWidth: (thumbnailListView.thumbnailWidth + hoverItem.margins.left + hoverItem.margins.right) * thumbnailListView.count
            property int optimalHeight: thumbnailListView.thumbnailWidth*(1.0/screenFactor) + hoverItem.margins.top + hoverItem.margins.bottom + PlasmaCore.Units.gridUnit * 2
            property bool canStretchX: false
            property bool canStretchY: false
            width: Math.min(Math.max(tabBox.screenGeometry.width * 0.3, optimalWidth), tabBox.screenGeometry.width * 0.9)
            height: Math.min(Math.max(tabBox.screenGeometry.height * 0.15, optimalHeight), tabBox.screenGeometry.height * 0.7)
            clip: true
            focus: true

            // just to get the margin sizes
            PlasmaCore.FrameSvgItem {
                id: hoverItem
                imagePath: "widgets/viewitem"
                prefix: "hover"
                visible: false
            }

            ListView {
                id: thumbnailListView
                model: tabBox.model
                orientation: ListView.Horizontal
                property int thumbnailWidth: 300 * PlasmaCore.Units.devicePixelRatio
                height: thumbnailWidth * (1.0/dialogMainItem.screenFactor) + hoverItem.margins.bottom + hoverItem.margins.top
                spacing: PlasmaCore.Units.smallSpacing
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                width: Math.min(parent.width - (anchors.leftMargin + anchors.rightMargin) - (hoverItem.margins.left + hoverItem.margins.right), thumbnailWidth * count + 5 * (count - 1))
                anchors {
                    top: parent.top
                    horizontalCenter: parent.horizontalCenter
                }
                clip: true
                delegate: Item {
                    property alias caption: thumbnailItem.caption
                    property alias icon: thumbnailItem.icon
                    id: delegateItem
                    width: thumbnailListView.thumbnailWidth
                    height: thumbnailListView.thumbnailWidth*(1.0/dialogMainItem.screenFactor)
                    Item {
                        property variant caption: model.caption
                        property variant icon: model.icon
                        id: thumbnailItem
                        anchors {
                            fill: parent
                            leftMargin: hoverItem.margins.left
                            rightMargin: hoverItem.margins.right
                            topMargin: hoverItem.margins.top
                            bottomMargin: hoverItem.margins.bottom
                        }
                        KWin.ThumbnailItem {
                            wId: windowId
                            clip: true
                            clipTo: thumbnailListView
                            anchors.fill: parent
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            thumbnailListView.currentIndex = index;
                        }
                    }
                }
                highlight: PlasmaCore.FrameSvgItem {
                    id: highlightItem
                    imagePath: "widgets/viewitem"
                    prefix: "hover"
                    width: thumbnailListView.thumbnailWidth
                    height: thumbnailListView.thumbnailWidth*(1.0/dialogMainItem.screenFactor)
                }
                boundsBehavior: Flickable.StopAtBounds
                Connections {
                    target: tabBox
                    function onCurrentIndexChanged() {thumbnailListView.currentIndex = tabBox.currentIndex;}
                }
            }
            Item {
                height: PlasmaCore.Units.gridUnit * 2
                id: captionFrame
                anchors {
                    top: thumbnailListView.bottom
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                    topMargin: hoverItem.margins.bottom
                }
                QIconItem {
                    id: iconItem
                    icon: thumbnailListView.currentItem ? thumbnailListView.currentItem.icon : ""
                    width: PlasmaCore.Units.iconSizes.medium
                    height: PlasmaCore.Units.iconSizes.medium
                    anchors {
                        verticalCenter: parent.verticalCenter
                        right: textItem.left
                        rightMargin: PlasmaCore.Units.smallSpacing
                    }
                }
                PlasmaComponents3.Label {
                    function constrainWidth() {
                        if (textItem.width > textItem.maxWidth && textItem.width > 0 && textItem.maxWidth > 0) {
                            textItem.width = textItem.maxWidth;
                        } else {
                            textItem.width = undefined;
                        }
                    }
                    function calculateMaxWidth() {
                        textItem.maxWidth = dialogMainItem.width - captionFrame.anchors.leftMargin - captionFrame.anchors.rightMargin - iconItem.width * 2 - captionFrame.anchors.rightMargin;
                    }
                    id: textItem
                    property int maxWidth: 0
                    text: thumbnailListView.currentItem ? thumbnailListView.currentItem.caption : ""
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideMiddle
                    font.weight: Font.Bold
                    anchors {
                        verticalCenter: parent.verticalCenter
                        horizontalCenter: parent.horizontalCenter
                    }
                    onTextChanged: textItem.constrainWidth()
                    Component.onCompleted: textItem.calculateMaxWidth()
                    Connections {
                        target: dialogMainItem
                        function onWidthChanged() {
                            textItem.calculateMaxWidth();
                            textItem.constrainWidth();
                        }
                    }
                }
            }
            /*
            * Key navigation on outer item for two reasons:
            * @li we have to emit the change signal
            * @li on multiple invocation it does not work on the list view. Focus seems to be lost.
            **/
            Keys.onPressed: {
                if (event.key == Qt.Key_Left) {
                    thumbnailListView.decrementCurrentIndex();
                } else if (event.key == Qt.Key_Right) {
                    thumbnailListView.incrementCurrentIndex();
                }
            }
        }
    }
}
