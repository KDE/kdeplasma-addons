/*
 KWin - the KDE window manager
 This file is part of the KDE project.

    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kwin 2.0 as KWin

KWin.Switcher {
    id: tabBox
    currentIndex: listView.currentIndex
    property real screenFactor: tabBox.screenGeometry.width/tabBox.screenGeometry.height

    PlasmaCore.Dialog {
        id: dialog
        location: PlasmaCore.Types.Floating
        visible: tabBox.visible
        flags: Qt.X11BypassWindowManagerHint
        x: tabBox.screenGeometry.x + tabBox.screenGeometry.width * 0.5 - dialogMainItem.width * 0.5
        y: tabBox.screenGeometry.y + tabBox.screenGeometry.height * 0.5 - dialogMainItem.height * 0.5

        mainItem: Item {
            id: dialogMainItem
            property bool allDesktops: true
            property int optimalWidth: (listView.thumbnailWidth + hoverItem.margins.left + hoverItem.margins.right) * listView.count
            property int optimalHeight: listView.thumbnailWidth*(1.0/screenFactor) + hoverItem.margins.top + hoverItem.margins.bottom + 40
            property bool canStretchX: true
            property bool canStretchY: false
            width: Math.min(Math.max(tabBox.screenGeometry.width * 0.2, optimalWidth), tabBox.screenGeometry.width * 0.8)
            height: Math.min(optimalHeight, tabBox.screenGeometry.height * 0.8)

            // just to get the margin sizes
            PlasmaCore.FrameSvgItem {
                id: hoverItem
                imagePath: "widgets/viewitem"
                prefix: "hover"
                visible: false
            }

            ListView {
                id: listView
                property int thumbnailWidth: 600
                height: thumbnailWidth * (1.0/screenFactor) + hoverItem.margins.bottom + hoverItem.margins.top
                width: Math.min(parent.width - (anchors.leftMargin + anchors.rightMargin) - (hoverItem.margins.left + hoverItem.margins.right), thumbnailWidth * count + 5 * (count - 1))
                spacing: 5
                orientation: ListView.Horizontal
                model: tabBox.model
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
                clip: true
                highlight: PlasmaCore.FrameSvgItem {
                    id: highlightItem
                    imagePath: "widgets/viewitem"
                    prefix: "hover"
                    width: listView.thumbnailWidth
                    height: listView.thumbnailWidth*(1.0/screenFactor)
                }
                delegate: Item {
                    property alias caption: thumbnailItem.caption
                    width: listView.thumbnailWidth
                    height: listView.thumbnailWidth*(1.0/screenFactor)
                    KWin.DesktopThumbnailItem {
                        id: thumbnailItem
                        property variant caption: model.caption
                        clip: true
                        clipTo: listView
                        desktop: model.desktop
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
                            listView.currentIndex = index;
                        }
                    }
                }
                highlightMoveDuration: PlasmaCore.Units.longDuration
                boundsBehavior: Flickable.StopAtBounds
                Connections {
                    target: tabBox
                    function onCurrentIndexChanged() {listView.currentIndex = tabBox.currentIndex;}
                }
            }
            Item {
                height: 40
                id: captionFrame
                anchors {
                    top: listView.bottom
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                    topMargin: hoverItem.margins.bottom
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
                        textItem.maxWidth = dialogMainItem.width - captionFrame.anchors.leftMargin - captionFrame.anchors.rightMargin - - captionFrame.anchors.rightMargin;
                    }
                    id: textItem
                    property int maxWidth: 0
                    text: listView.currentItem ? listView.currentItem.caption : ""
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
        }
    }
}
