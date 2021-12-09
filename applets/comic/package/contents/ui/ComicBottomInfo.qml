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

    implicitWidth: 10
    implicitHeight: comicIdentifier.height

    property bool showUrl: false
    property bool showIdentifier: false
    property variant comicData

    visible: (comicIdentifier.text.length > 0 || comicUrl.text.length > 0)

    PlasmaComponents3.Label {
        id: comicIdentifier

        anchors {
            left: root.left
            top: root.top
            bottom: root.bottom
            right: comicUrl.left
            leftMargin: 2
        }

        color: PlasmaCore.Theme.textColor
        visible: (showIdentifier && comicIdentifier.text.length > 0)
        text: (showIdentifier && comicData.currentReadable != undefined) ? comicData.currentReadable : ""

        MouseArea {
            id: idLabelArea

            anchors.fill: parent

            hoverEnabled: true

            onEntered: {
                parent.color = PlasmaCore.Theme.highlightColor;
            }

            onExited: {
                parent.color = PlasmaCore.Theme.textColor;
            }

            onClicked: {
                plasmoid.nativeInterface.goJump();
            }

            PlasmaCore.ToolTipArea {
                anchors.fill: idLabelArea
                mainText: i18nc("@info:tooltip", "Jump to strip…")
            }
        }
    }

    PlasmaComponents3.Label {
        id:comicUrl

        anchors {
            top: root.top
            bottom: root.bottom
            right: root.right
            rightMargin: 2
        }

        color: PlasmaCore.Theme.textColor
        visible: (showUrl && comicUrl.text.length > 0)
        text: (showUrl && comicData.websiteHost.length > 0) ? comicData.websiteHost : ""

        MouseArea {
            id: idUrlLabelArea

            anchors.fill: parent

            hoverEnabled: true

            onEntered: {
                parent.color = PlasmaCore.Theme.highlightColor;
            }

            onExited: {
                parent.color = PlasmaCore.Theme.textColor;
            }

            onClicked: {
                plasmoid.nativeInterface.shop();
            }

            PlasmaCore.ToolTipArea {
                anchors.fill: idUrlLabelArea
                mainText: i18nc("@info:tooltip", "Visit the comic website")
            }
        }
    }
}
