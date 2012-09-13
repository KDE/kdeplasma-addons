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

Rectangle {
    id: root

    signal prevClicked
    signal nextClicked
    signal zoomClicked

    width: prevButton.width*3 +10
    height: prevButton.height+10
    color: "#bb000000"
    border {
        color: "black"
        width: 2
    }
    radius: 3
    anchors {
        horizontalCenter: parent.horizontalCenter
        bottom: parent.bottom
        bottomMargin:10
    }

    Component.onCompleted: {
        prevButton.clicked.connect(root.prevClicked);
        nextButton.clicked.connect(root.nextClicked);
        zoomButton.clicked.connect(root.zoomClicked);
    }

    Row {
        anchors.centerIn: parent

        PlasmaComponents.ToolButton {
            id: prevButton
            iconSource: "arrow-left"
            enabled: comicData.hasPrev()
        }

        PlasmaComponents.ToolButton {
            id: zoomButton
            iconSource: "zoom-original"
        }

        PlasmaComponents.ToolButton {
            id: nextButton
            iconSource: "arrow-right"
            enabled: comicData.hasNext()
        }
    }
}