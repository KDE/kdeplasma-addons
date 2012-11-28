/*
 * Copyright 2012  Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.qtextracomponents 0.1

PlasmaExtras.ScrollArea {
    id: root

    width: comicPicture.nativeWidth
    height: comicPicture.nativeHeight

    property bool actualSize: false
    property bool isLeftToRight: true
    property bool isTopToBottom: true

    property alias image: comicPicture.image
    property alias tooltipText: tooltip.mainText

    function calculateContentWidth() {
        return actualSize ? (comicPicture.nativeWidth > viewContainer.width ? comicPicture.nativeWidth : viewContainer.width) : viewContainer.width;
    }

    function calculateContentHeight() {
        return actualSize ? (comicPicture.nativeHeight > viewContainer.height ? comicPicture.nativeHeight : viewContainer.height) : viewContainer.height;
    }

    Flickable {
        id: viewContainer

        anchors.fill:parent

        contentWidth: comicPictureHolder.width
        contentHeight: comicPictureHolder.height

        clip: true

        Item {
            id: comicPictureHolder

            width: Math.max(comicPicture.width, viewContainer.width);
            height: Math.max(comicPicture.height, viewContainer.height);

            QImageItem {
                id: comicPicture

                anchors.centerIn: parent

                width: actualSize ? comicPicture.nativeWidth : viewContainer.width
                height: actualSize ? comicPicture.nativeHeight : viewContainer.height

                onImageChanged: {
                    viewContainer.contentX = (root.isLeftToRight) ? 0 : ( viewContainer.contentWidth - viewContainer.width);
                    viewContainer.contentY = (root.isTopToBottom) ? 0 : ( viewContainer.contentHeight - viewContainer.height);
                }

                smooth: true
                fillMode: QImageItem.PreserveAspectFit
            }

            MouseArea {
                id:mouseArea

                anchors.fill: parent

                hoverEnabled: true
                preventStealing: false
                acceptedButtons: Qt.LeftButton | Qt.MiddleButton

                onClicked: {
                    if (mouse.button == Qt.MiddleButton && comicApplet.middleClick) {
                        fullDialog.open();
                    }
                }

                PlasmaCore.ToolTip {
                    id: tooltip
                    //target: root
                }

                ButtonBar {
                    id: buttonBar

                    pos {
                        y: viewContainer.height - buttonBar.height + viewContainer.contentY
                        x: (viewContainer.width - buttonBar.width)/2 + viewContainer.contentX
                    }

                    opacity: 0

                    onPrevClicked: {
                        //busyIndicator.visible = true;
                        comicApplet.updateComic(comicData.prev);
                    }

                    onNextClicked: {
                        //busyIndicator.visible = true;
                        comicApplet.updateComic(comicData.next);
                    }

                    onZoomClicked: {
                        fullDialog.open();
                    }

                    states: State {
                        name: "show"; when: (comicApplet.arrowsOnHover && mouseArea.containsMouse)
                        PropertyChanges { target: buttonBar; opacity: 1; }
                    }

                    transitions: Transition {
                        from: ""; to: "show"; reversible: true
                        NumberAnimation { properties: "opacity"; duration: 250; easing.type: Easing.InOutQuad }
                    }
                }
            }
        }
    }
}
