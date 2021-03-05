/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: root

    width: PlasmaCore.Units.gridUnit
    height: PlasmaCore.Units.gridUnit

    property variant comicData

    PlasmaComponents3.ToolButton {
        id: arrowLeft

        anchors {
            left: root.left
            verticalCenter: root.verticalCenter
        }

        icon.name: "go-previous"
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
                NumberAnimation { properties: "opacity"; duration: PlasmaCore.Units.longDuration; easing.type: Easing.InOutQuad }
            }
        }
    }

    PlasmaComponents3.ToolButton {
        id: arrowRight

        anchors {
            right: root.right
            verticalCenter: root.verticalCenter
        }

        icon.name: "go-next"
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
