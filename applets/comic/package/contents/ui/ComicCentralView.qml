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
    property variant comicData

    id: root
    width: 10
    height: 10
    color: "transparent"

    ActionButton {
        id: arrowLeft
        svg: arrowsSvg
        elementId: "left-arrow"
        width: 48
        height: 48
        anchors {
            left: root.left
            verticalCenter: root.verticalCenter
        }
        visible: (!comicApplet.arrowsOnHover && (comicData.prev.length > 0))
        onClicked: {
            //busyIndicator.visible = true;
            comicApplet.updateComic(comicData.prev);
        }
    }

    ImageWidget {
        id: comicImage
        image: comicApplet.comicData.image
        tooltipText: comicApplet.comicData.additionalText
        fullView: comicApplet.showActualSize
        anchors { 
            left: arrowLeft.visible ? arrowLeft.right : root.left
            right: arrowRight.visible ? arrowRight.left : root.right
            top: root.top
            bottom: root.bottom
        }
    }

    ActionButton {
        id: arrowRight
        svg: arrowsSvg
        elementId: "right-arrow"
        width: 48
        height: 48
        anchors {
            right: root.right
            verticalCenter: root.verticalCenter
        }
        visible: (!comicApplet.arrowsOnHover && (comicData.next.length > 0))
        onClicked: {
            //busyIndicator.visible = true;
            comicApplet.updateComic(comicData.next);
        }
    }

    FullViewWidget {
        id: fullDialog
        image: comicApplet.comicData.image
    }
}
