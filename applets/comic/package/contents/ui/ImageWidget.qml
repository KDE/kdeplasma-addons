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
import org.kde.qtextracomponents 0.1

Rectangle {
    id: imageWidget
    property alias image: comicPicture.image
    property bool fullView: false
    property alias tooltipText: tooltip.mainText
    color: "transparent"

    Flickable {
        id: viewContainer
        anchors {
            top: imageWidget.top
            left: imageWidget.left
            right: verticalScrollBar.left
            bottom: horizontalScrollBar.top
            bottomMargin: 4
            leftMargin: 4
        }
        contentWidth: fullView ? comicPicture.nativeWidth : viewContainer.width
        contentHeight: fullView ? comicPicture.nativeHeight : viewContainer.height
        clip: true
        
        QImageItem {
            id: comicPicture
            anchors { 
                left: parent.left
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }
            smooth: true
            //opacity: busyIndicator.visible ? 0.3 : 1.0
            fillMode: QImageItem.PreserveAspectFit
        }
    }

    PlasmaComponents.ScrollBar {
        id: horizontalScrollBar
        orientation: Qt.Horizontal
        anchors {
            bottom: imageWidget.bottom
            left: imageWidget.left
        }
        flickableItem: viewContainer
    }

    PlasmaComponents.ScrollBar {
        id: verticalScrollBar
        orientation: Qt.Vertical
        anchors {
            top: imageWidget.top
            right: imageWidget.right
        }
        flickableItem: viewContainer
    }

    MouseArea {
        id:mouseArea
        anchors.fill: viewContainer
        hoverEnabled: true
        preventStealing: false
        acceptedButtons: Qt.LeftButton | Qt.MiddleButton

        PlasmaCore.ToolTip {
            id: tooltip
            target: mouseArea
        }
        onClicked: {
            if (mouse.button == Qt.MiddleButton) {
                fullView = !fullView;
            }
        }

        ButtonBar {
            id: buttonBar
            visible: (comicApplet.arrowsOnHover && mouseArea.containsMouse)
            opacity: 0
            states: State {
                name: "show"; when: (comicApplet.arrowsOnHover && mouseArea.containsMouse)
                PropertyChanges { target: buttonBar; opacity: 1; }
            }

            transitions: Transition {
                from: ""; to: "show"; reversible: true
                NumberAnimation { properties: "opacity"; duration: 250; easing.type: Easing.InOutQuad }
            }

            onPrevClicked: {
                console.log("Previous clicked");
                //busyIndicator.visible = true;
                comicApplet.updateComic(comicData.prev);
            }
            onNextClicked: {
                console.log("Next clicked");
                //busyIndicator.visible = true;
                comicApplet.updateComic(comicData.next);
            }
            onZoomClicked: {
                //comicApplet.showFullView();
                fullDialog.open();
            }
        }
    }
    
}