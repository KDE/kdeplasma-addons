/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.1
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

PlasmaComponents.ScrollView {
    id: root

    width: comicPicture.nativeWidth
    height: comicPicture.nativeHeight

    property bool actualSize: false
    property bool isLeftToRight: true
    property bool isTopToBottom: true

    property alias image: comicPicture.image

    Flickable {
        id: viewContainer

        anchors.fill:parent

        contentWidth: comicPicture.width
        contentHeight: comicPicture.height
        clip: true

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
