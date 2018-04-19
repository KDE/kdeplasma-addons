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

    implicitWidth: 10
    implicitHeight: comicIdentifier.height

    property bool showUrl: false
    property bool showIdentifier: false
    property variant comicData

    visible: (comicIdentifier.text.length > 0 || comicUrl.text.length > 0)

    PlasmaComponents.Label {
        id: comicIdentifier

        anchors {
            left: root.left
            top: root.top
            bottom: root.bottom
            right: comicUrl.left
            leftMargin: 2
        }

        color: theme.textColor
        visible: (showIdentifier && comicIdentifier.text.length > 0)
        text: (showIdentifier && comicData.currentReadable != undefined) ? comicData.currentReadable : ""

        MouseArea {
            id: idLabelArea

            anchors.fill: parent

            hoverEnabled: true

            onEntered: {
                parent.color = theme.highlightColor;
            }

            onExited: {
                parent.color = theme.textColor;
            }

            onClicked: {
                plasmoid.nativeInterface.goJump();
            }

            PlasmaCore.ToolTipArea {
                anchors.fill: idLabelArea
                mainText: i18nc("@info:tooltip", "Jump to strip...")
            }
        }
    }

    PlasmaComponents.Label {
        id:comicUrl

        anchors {
            top: root.top
            bottom: root.bottom
            right: root.right
            rightMargin: 2
        }

        color: theme.textColor
        visible: (showUrl && comicUrl.text.length > 0)
        text: (showUrl && comicData.websiteHost.length > 0) ? comicData.websiteHost : ""

        MouseArea {
            id: idUrlLabelArea

            anchors.fill: parent

            hoverEnabled: true

            onEntered: {
                parent.color = theme.highlightColor;
            }

            onExited: {
                parent.color = theme.textColor;
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
