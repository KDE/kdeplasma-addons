/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3
import plasma.applet.org.kde.plasma.comic as Comic

Item {
    id: root

    implicitWidth: rowButton.width + background.margins.left + background.margins.right
    implicitHeight: rowButton.height + background.margins.top + background.margins.bottom

    signal prevClicked
    signal nextClicked
    signal zoomClicked

    KSvg.FrameSvgItem {
        id: background

        anchors.fill: parent

        imagePath: "widgets/toolbar"
        prefix: "raised"
    }

    Row {
        id: rowButton

        x: background.margins.left
        y: background.margins.top

        spacing: 4
        //ToolButton or Button in C++ use PushButton?
        PlasmaComponents3.Button {
            id: prevButton

            icon.name: "arrow-left"
            enabled: (comicData.prev != undefined && comicData.prev.length > 0)
            onClicked: root.prevClicked()
        }

        PlasmaComponents3.Button {
            id: zoomButton

            icon.name: "zoom-original"
            onClicked: root.zoomClicked()
        }

        PlasmaComponents3.Button {
            id: nextButton

            icon.name: "arrow-right"
            enabled: (comicData.next != undefined && comicData.next.length > 0)
            onClicked: root.nextClicked()
        }
    }
}
