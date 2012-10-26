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

Item {
    id: root

    signal prevClicked
    signal nextClicked
    signal zoomClicked

    width: prevButton.width*3 + 10 + 14
    height: prevButton.height + 20
    
    Component.onCompleted: {
        prevButton.clicked.connect(root.prevClicked);
        nextButton.clicked.connect(root.nextClicked);
        zoomButton.clicked.connect(root.zoomClicked);
    }

    PlasmaCore.FrameSvgItem {
        id: myFrameSvgItem
        anchors.fill: parent
        imagePath: "widgets/viewitem"
        prefix: "hover"
    }
    
    Row {
        spacing: 4
        anchors.centerIn: parent
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
}