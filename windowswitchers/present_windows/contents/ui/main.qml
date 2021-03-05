/*
 KWin - the KDE window manager
 This file is part of the KDE project.

 SPDX-FileCopyrightText: 2012 Martin Gräßlin <mgraesslin@kde.org>

 SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Layouts 1.1
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
            property int optimalWidth: 0.9*tabBox.screenGeometry.width
            property int optimalHeight: 0.9*tabBox.screenGeometry.height
            property int standardMargin: PlasmaCore.Units.smallSpacing
            width: optimalWidth
            height: optimalHeight
            focus: true

            // just to get the margin sizes
            PlasmaCore.FrameSvgItem {
                id: hoverItem
                imagePath: "widgets/viewitem"
                prefix: "hover"
                visible: false
            }

            GridView {
                property int rows: Math.round(Math.sqrt(count))
                property int columns: (rows*rows < count) ? rows + 1 : rows
                id: thumbnailListView
                model: tabBox.model
                cellWidth: Math.floor(width / columns)
                cellHeight: Math.floor(height / rows)
                clip: true
                anchors {
                    fill: parent
                }
                delegate: Item {
                    width: thumbnailListView.cellWidth
                    height: thumbnailListView.cellHeight
                    Item {
                        id: thumbnailItem
                        anchors {
                            top: parent.top
                            left: parent.left
                            right: parent.right
                            bottom: captionItem.top
                            leftMargin: hoverItem.margins.left
                            rightMargin: hoverItem.margins.right
                            topMargin: hoverItem.margins.top
                            bottomMargin: dialogMainItem.standardMargin
                        }
                        KWin.ThumbnailItem {
                            wId: windowId
                            clip: true
                            clipTo: thumbnailListView
                            anchors.fill: parent
                        }
                    }
                    RowLayout {
                        id: captionItem
                        height: childrenRect.height
                        property int maximumWidth: parent.width - hoverItem.margins.left - dialogMainItem.standardMargin - hoverItem.margins.right
                        Layout.maximumWidth: captionItem.maximumWidth
                        anchors {
                            bottom: parent.bottom
                            bottomMargin: hoverItem.margins.bottom
                            horizontalCenter: parent.horizontalCenter
                        }
                        QIconItem {
                            id: iconItem
                            icon: model.icon
                            property int iconSize: PlasmaCore.Units.iconSizes.medium
                            width: iconSize
                            height: iconSize
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                            Layout.preferredWidth: iconSize
                            Layout.preferredHeight: iconSize
                        }
                        PlasmaComponents3.Label {
                            text: caption
                            font.italic: minimized
                            font.weight: index === thumbnailListView.currentIndex ? Font.Bold : Font.Normal
                            elide: Text.ElideMiddle
                            Layout.fillWidth: true
                            Layout.maximumWidth: captionItem.maximumWidth - iconItem.iconSize - captionItem.spacing * 2
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
                    width: thumbnailListView.cellWidth
                    height: thumbnailListView.cellHeight
                }
                highlightMoveDuration: 0
                boundsBehavior: Flickable.StopAtBounds
                Connections {
                    target: tabBox
                    function onCurrentIndexChanged() {thumbnailListView.currentIndex = tabBox.currentIndex;}
                }
            }
            /*
            * Key navigation on outer item for two reasons:
            * @li we have to emit the change signal
            * @li on multiple invocation it does not work on the list view. Focus seems to be lost.
            **/
            Keys.onPressed: {
                if (event.key == Qt.Key_Left) {
                    thumbnailListView.moveCurrentIndexLeft();
                } else if (event.key == Qt.Key_Right) {
                    thumbnailListView.moveCurrentIndexRight();
                } else if (event.key == Qt.Key_Up) {
                    thumbnailListView.moveCurrentIndexUp();
                } else if (event.key == Qt.Key_Down) {
                    thumbnailListView.moveCurrentIndexDown();
                }
            }
        }
    }
}
