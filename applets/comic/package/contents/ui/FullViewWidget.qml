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
import QtQuick.Layouts 1.1


PlasmaCore.Dialog {
    id: root

    property alias image: comicPicture.image

    flags: Qt.Popup
    visible: false

    function toggleVisibility()
    {
        root.visible = !root.visible;
        if (root.visible) {
            plasmoid.nativeInterface.positionFullView(root);
            root.activateWindow;
        }
    }

    function close() {
        root.visible = false;
    }

    mainItem: PlasmaExtras.ScrollArea {
        id: mainScrollArea

        anchors.fill: parent

        Layout.minimumWidth: comicPicture.nativeWidth
        Layout.maximumWidth: Layout.minimumWidth
        Layout.minimumHeight: comicPicture.nativeHeight
        Layout.maximumHeight: Layout.minimumHeight

        Flickable {
            id: viewContainer

            anchors.fill: parent

            contentWidth: comicPicture.nativeWidth
            contentHeight: comicPicture.nativeHeight

            //clip: true

            QImageItem {
                id: comicPicture

                anchors.fill: parent

                smooth: true
                fillMode: QImageItem.PreserveAspectFit

                MouseArea {
                    id: dialogMouseArea

                    anchors.fill: comicPicture

                    onClicked: {
                        root.close();
                    }
                }
            }
        }
    }
}
