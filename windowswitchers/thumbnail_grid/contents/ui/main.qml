/*
 KWin - the KDE window manager
 This file is part of the KDE project.

 SPDX-FileCopyrightText: 2020 Chris Holland <zrenfire@gmail.com>

 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kwin 3.0 as KWin
import org.kde.kirigami 2.20 as Kirigami

// https://techbase.kde.org/Development/Tutorials/KWin/WindowSwitcher
// https://github.com/KDE/kwin/blob/master/tabbox/switcheritem.h
KWin.TabBoxSwitcher {
    id: tabBox
    currentIndex: thumbnailGridView.currentIndex

    PlasmaCore.Dialog {
        id: dialog
        location: PlasmaCore.Types.Floating
        visible: tabBox.visible
        flags: Qt.X11BypassWindowManagerHint
        x: tabBox.screenGeometry.x + tabBox.screenGeometry.width * 0.5 - dialogMainItem.width * 0.5
        y: tabBox.screenGeometry.y + tabBox.screenGeometry.height * 0.5 - dialogMainItem.height * 0.5

        mainItem: Item {
            id: dialogMainItem

            focus: true

            property int maxWidth: tabBox.screenGeometry.width * 0.9
            property int maxHeight: tabBox.screenGeometry.height * 0.7
            property real screenFactor: tabBox.screenGeometry.width / tabBox.screenGeometry.height
            property int maxGridColumnsByWidth: Math.floor(maxWidth / thumbnailGridView.cellWidth)

            property int gridColumns: {         // Simple greedy algorithm
                // respect screenGeometry
                const c = Math.min(thumbnailGridView.count, maxGridColumnsByWidth);
                const residue = thumbnailGridView.count % c;
                if (residue == 0) {
                    return c;
                }
                // start greedy recursion
                return columnCountRecursion(c, c, c - residue);
            }

            property int gridRows: Math.ceil(thumbnailGridView.count / gridColumns)
            property int optimalWidth: thumbnailGridView.cellWidth * gridColumns
            property int optimalHeight: thumbnailGridView.cellHeight * gridRows
            width: Math.min(Math.max(thumbnailGridView.cellWidth, optimalWidth), maxWidth)
            height: Math.min(Math.max(thumbnailGridView.cellHeight, optimalHeight), maxHeight)

            clip: true

            // Step for greedy algorithm
            function columnCountRecursion(prevC, prevBestC, prevDiff) {
                const c = prevC - 1;

                // don't increase vertical extent more than horizontal
                // and don't exceed maxHeight
                if (prevC * prevC <= thumbnailGridView.count + prevDiff ||
                        maxHeight < Math.ceil(thumbnailGridView.count / c) * thumbnailGridView.cellHeight) {
                    return prevBestC;
                }
                const residue = thumbnailGridView.count % c;
                // halts algorithm at some point
                if (residue == 0) {
                    return c;
                }
                // empty slots
                const diff = c - residue;

                // compare it to previous count of empty slots
                if (diff < prevDiff) {
                    return columnCountRecursion(c, c, diff);
                } else if (diff == prevDiff) {
                    // when it's the same try again, we'll stop early enough thanks to the landscape mode condition
                    return columnCountRecursion(c, prevBestC, diff);
                }
                // when we've found a local minimum choose this one (greedy)
                return columnCountRecursion(c, prevBestC, diff);
            }

            // Just to get the margin sizes
            PlasmaCore.FrameSvgItem {
                id: hoverItem
                imagePath: "widgets/viewitem"
                prefix: "hover"
                visible: false
            }

            GridView {
                id: thumbnailGridView
                anchors.fill: parent

                model: tabBox.model

                property int iconSize: PlasmaCore.Units.iconSizes.huge
                property int captionRowHeight: 30 * PlasmaCore.Units.devicePixelRatio // The close button is 30x30 in Breeze
                property int thumbnailWidth: 300 * PlasmaCore.Units.devicePixelRatio
                property int thumbnailHeight: thumbnailWidth * (1.0/dialogMainItem.screenFactor)
                cellWidth: hoverItem.margins.left + thumbnailWidth + hoverItem.margins.right
                cellHeight: hoverItem.margins.top + captionRowHeight + thumbnailHeight + hoverItem.margins.bottom

                keyNavigationWraps: true
                highlightMoveDuration: 0

                delegate: Item {
                    id: thumbnailGridItem
                    width: thumbnailGridView.cellWidth
                    height: thumbnailGridView.cellHeight
                    readonly property bool isCurrentItem: GridView.isCurrentItem

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            thumbnailGridItem.select();
                        }
                    }
                    function select() {
                        thumbnailGridView.currentIndex = index;
                        thumbnailGridView.currentIndexChanged(thumbnailGridView.currentIndex);
                    }

                    ColumnLayout {
                        id: columnLayout
                        z: 0
                        spacing: PlasmaCore.Units.largeSpacing
                        anchors.fill: parent
                        anchors.leftMargin: hoverItem.margins.left
                        anchors.topMargin: hoverItem.margins.top
                        anchors.rightMargin: hoverItem.margins.right
                        anchors.bottomMargin: hoverItem.margins.bottom


                        // KWin.WindowThumbnail needs a container
                        // otherwise it will be drawn the same size as the parent ColumnLayout
                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            KWin.WindowThumbnail {
                                anchors.fill: parent
                                wId: windowId
                            }

                            PlasmaCore.IconItem {
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.verticalCenter: parent.bottom
                                anchors.verticalCenterOffset: Math.round(-columnLayout.spacing/2)
                                width: thumbnailGridView.iconSize
                                height: thumbnailGridView.iconSize

                                source: model.icon
                                usesPlasmaTheme: false
                            }

                            PlasmaComponents3.Button {
                                id: closeButton
                                anchors {
                                    right: parent.right
                                    top: parent.top
                                    margins: PlasmaCore.Units.smallSpacing
                                }
                                visible: model.closeable && typeof tabBox.model.close !== 'undefined' &&
                                        (mouseArea.containsMouse
                                         || closeButton.hovered
                                         || thumbnailGridItem.isCurrentItem
                                         || Kirigami.Settings.tabletMode
                                         || Kirigami.Settings.hasTransientTouchInput
                                        )
                                icon.name: 'window-close-symbolic'
                                onClicked: {
                                    tabBox.model.close(index);
                                }
                            }
                        }

                        PlasmaComponents3.Label {
                            Layout.fillWidth: true
                            text: model.caption
                            horizontalAlignment: Text.AlignHCenter
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                        }
                    }
                } // GridView.delegate

                highlight: PlasmaCore.FrameSvgItem {
                    id: highlightItem
                    imagePath: "widgets/viewitem"
                    prefix: "hover"
                }

                Connections {
                    target: tabBox
                    function onCurrentIndexChanged() {
                        thumbnailGridView.currentIndex = tabBox.currentIndex;
                    }
                }
            } // GridView

            Keys.onPressed: {
                if (event.key == Qt.Key_Left) {
                    thumbnailGridView.moveCurrentIndexLeft();
                } else if (event.key == Qt.Key_Right) {
                    thumbnailGridView.moveCurrentIndexRight();
                } else if (event.key == Qt.Key_Up) {
                    thumbnailGridView.moveCurrentIndexUp();
                } else if (event.key == Qt.Key_Down) {
                    thumbnailGridView.moveCurrentIndexDown();
                } else {
                    return;
                }

                thumbnailGridView.currentIndexChanged(thumbnailGridView.currentIndex);
            }
        } // Dialog.mainItem
    } // Dialog
}
