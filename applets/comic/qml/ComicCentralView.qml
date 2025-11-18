/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid
import plasma.applet.org.kde.plasma.comic as Comic

Item {
    id: root

    property Comic.comicData comicData
    signal updateComicRequested(comicId: string)

    PlasmaComponents3.ToolButton {
        id: arrowLeft

        anchors {
            left: root.left
            verticalCenter: root.verticalCenter
        }

        icon.name: "go-previous"
        visible: (!Plasmoid.configuration.arrowsOnHover && (root.comicData.prev !== undefined))

        onClicked: {
            root.updateComicRequested(root.comicData.prev);
        }
    }
    PlasmaCore.ToolTipArea {
        id: tooltip
        anchors {
            left: arrowLeft.visible ? arrowLeft.right : root.left
            right: arrowRight.visible ? arrowRight.left : root.right
            leftMargin: arrowLeft.visible ? 4 : 0
            rightMargin: arrowRight.visible ? 4 : 0
            top: root.top
            bottom: root.bottom
        }
        subText: root.comicData.additionalText ?? ""
        active: subText

        MouseArea {
            id: comicImageArea

            anchors.fill: parent

            hoverEnabled: true
            preventStealing: false
            acceptedButtons: Qt.LeftButton | Qt.MiddleButton

            onClicked: mouse => {
                if (mouse.button == Qt.MiddleButton && Plasmoid.configuration.middleClick) {
                    fullDialog.toggleVisibility();
                }
            }

            ImageWidget {
                id: comicImage

                anchors.fill: parent
                enabled: false
                visible: !root.comicData.isError ?? true

                image: root.comicData.image
                actualSize: Plasmoid.showActualSize
                isLeftToRight: root.comicData.isLeftToRight ?? false
                isTopToBottom: root.comicData.isTopToBottom ?? false
            }

            ErrorPlaceholder {
                id: errorPlaceholder
                // intentionally not checking width, it will prevent attempts to fit with word wrap
                property bool fitsInWidget: implicitHeight <= comicImageArea.height
                anchors.centerIn: parent
                width: parent.width
                height: parent.height
                visible: (root.comicData.isError && fitsInWidget) ?? false
                identifierType: root.comicData.type ?? Comic.comicData.StringIdentifier
                identifier: root.comicData.current
            }

            PlasmaExtras.PlaceholderMessage {
                id: tinyErrorPlaceholder
                anchors.centerIn: parent
                width: parent.width
                height: parent.height
                visible: (root.comicData.isError && !errorPlaceholder.visible) ?? false
                iconName: "error-symbolic"
            }


            ButtonBar {
                id: buttonBar

                comicData: root.comicData

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.bottom
                    bottomMargin: 10
                }

                visible: Plasmoid.configuration.arrowsOnHover && comicImageArea.containsMouse
                opacity: 0

                onPrevClicked: {
                    root.updateComicRequested(root.comicData.prev);
                }

                onNextClicked: {
                    root.updateComicRequested(root.comicData.next);
                }

                onZoomClicked: {
                    fullDialog.toggleVisibility();
                }

                states: State {
                    name: "show"; when: (Plasmoid.configuration.arrowsOnHover && comicImageArea.containsMouse)
                    PropertyChanges { buttonBar.opacity: 1 }
                }

                transitions: Transition {
                    from: ""; to: "show"; reversible: true
                    NumberAnimation { properties: "opacity"; duration: Kirigami.Units.longDuration; easing.type: Easing.InOutQuad }
                }
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
        visible: (!Plasmoid.configuration.arrowsOnHover && (root.comicData.next !== undefined))

        onClicked: {
            root.updateComicRequested(root.comicData.next);
        }
    }

    FullViewWidget {
        id: fullDialog

        image: root.comicData.image
        isError: root.comicData.isError ?? false
        identifier: root.comicData.current
        identifierType: root.comicData.type ?? Comic.comicData.StringIdentifier
    }
}
