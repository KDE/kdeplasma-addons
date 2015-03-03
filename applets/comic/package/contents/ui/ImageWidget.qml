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
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

PlasmaExtras.ScrollArea {
    id: root

    width: comicPicture.nativeWidth
    height: comicPicture.nativeHeight

    property bool actualSize: false
    property bool isLeftToRight: true
    property bool isTopToBottom: true

    property alias image: comicPicture.image

    function calculateContentWidth() {
        return actualSize ? (comicPicture.nativeWidth > viewContainer.width ? comicPicture.nativeWidth : viewContainer.width) : viewContainer.width;
    }

    function calculateContentHeight() {
        return actualSize ? (comicPicture.nativeHeight > viewContainer.height ? comicPicture.nativeHeight : viewContainer.height) : viewContainer.height;
    }

    Flickable {
        id: viewContainer

        anchors.fill:parent

        contentWidth: comicPictureHolder.width
        contentHeight: comicPictureHolder.height

        clip: true

        Item {
            id: comicPictureHolder

            width: Math.max(comicPicture.width, viewContainer.width);
            height: Math.max(comicPicture.height, viewContainer.height);

            QImageItem {
                id: comicPicture

                anchors.centerIn: parent

                width: actualSize ? comicPicture.nativeWidth : viewContainer.width
                height: actualSize ? comicPicture.nativeHeight : viewContainer.height

                onImageChanged: {
                    viewContainer.contentX = (root.isLeftToRight) ? 0 : ( viewContainer.contentWidth - viewContainer.width);
                    viewContainer.contentY = (root.isTopToBottom) ? 0 : ( viewContainer.contentHeight - viewContainer.height);
                }

                smooth: true
                fillMode: QImageItem.PreserveAspectFit
            }
        }
    }
}
