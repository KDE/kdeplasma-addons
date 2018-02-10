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

Item {
    id: root

    implicitWidth: rowButton.width + background.margins.left + background.margins.right
    implicitHeight: rowButton.height + background.margins.top + background.margins.bottom

    signal prevClicked
    signal nextClicked
    signal zoomClicked

    PlasmaCore.FrameSvgItem {
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
        PlasmaComponents.Button {
            id: prevButton

            iconSource: "arrow-left"
            enabled: (comicData.prev != undefined && comicData.prev.length > 0)
        }

        PlasmaComponents.Button {
            id: zoomButton

            iconSource: "zoom-original"
        }

        PlasmaComponents.Button {
            id: nextButton

            iconSource: "arrow-right"
            enabled: (comicData.next != undefined && comicData.next.length > 0)
        }
    }

    Component.onCompleted: {
        prevButton.clicked.connect(root.prevClicked);
        nextButton.clicked.connect(root.nextClicked);
        zoomButton.clicked.connect(root.zoomClicked);
    }
}
