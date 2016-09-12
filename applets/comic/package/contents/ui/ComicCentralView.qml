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

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: root

    width: units.gridUnit
    height: units.gridUnit

    property variant comicData

    PlasmaComponents.ToolButton {
        id: arrowLeft

        anchors {
            left: root.left
            verticalCenter: root.verticalCenter
        }

        iconSource: "go-previous"
        visible: (!plasmoid.nativeInterface.arrowsOnHover && (comicData.prev !== undefined))

        onClicked: {
            plasmoid.nativeInterface.updateComic(comicData.prev);
        }
    }

    MouseArea {
        id: comicImageArea

        anchors {
            left: arrowLeft.visible ? arrowLeft.right : root.left
            right: arrowRight.visible ? arrowRight.left : root.right
            leftMargin: arrowLeft.visible ? 4 : 0
            rightMargin: arrowRight.visible ? 4 : 0
            top: root.top
            bottom: root.bottom
        }

        hoverEnabled: true
        preventStealing: false
        acceptedButtons: Qt.LeftButton | Qt.MiddleButton

        onClicked: {
            if (mouse.button == Qt.MiddleButton && plasmoid.nativeInterface.middleClick) {
                fullDialog.toggleVisibility();
            }
        }

        PlasmaCore.ToolTipArea {
            id: tooltip
            anchors.fill: comicImageArea
            subText: plasmoid.nativeInterface.comicData.additionalText
            active: subText
        }

        ImageWidget {
            id: comicImage

            anchors.fill: parent

            image: plasmoid.nativeInterface.comicData.image
            actualSize: plasmoid.nativeInterface.showActualSize
            isLeftToRight: plasmoid.nativeInterface.comicData.isLeftToRight
            isTopToBottom: plasmoid.nativeInterface.comicData.isTopToBottom
        }

        ButtonBar {
            id: buttonBar

            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                bottomMargin: 10
            }

            visible: plasmoid.nativeInterface.arrowsOnHover && comicImageArea.containsMouse
            opacity: 0

            onPrevClicked: {
                plasmoid.nativeInterface.updateComic(comicData.prev);
            }

            onNextClicked: {
                plasmoid.nativeInterface.updateComic(comicData.next);
            }

            onZoomClicked: {
                fullDialog.toggleVisibility();
            }

            states: State {
                name: "show"; when: (plasmoid.nativeInterface.arrowsOnHover && comicImageArea.containsMouse)
                PropertyChanges { target: buttonBar; opacity: 1; }
            }

            transitions: Transition {
                from: ""; to: "show"; reversible: true
                NumberAnimation { properties: "opacity"; duration: 250; easing.type: Easing.InOutQuad }
            }
        }
    }

    PlasmaComponents.ToolButton {
        id: arrowRight

        anchors {
            right: root.right
            verticalCenter: root.verticalCenter
        }

        iconSource: "go-next"
        visible: (!plasmoid.nativeInterface.arrowsOnHover && (comicData.next !== undefined))

        onClicked: {
            plasmoid.nativeInterface.updateComic(comicData.next);
        }
    }

    FullViewWidget {
        id: fullDialog

        image: plasmoid.nativeInterface.comicData.image
    }
}
