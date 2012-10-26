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
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.qtextracomponents 0.1

PlasmaCore.Dialog {
    property alias image: comicPicture.image
    
    id: fullDialog
    windowFlags: Qt.Popup
    visible: false
    mainItem: PlasmaExtras.ScrollArea {
        id: mainScrollArea
        width: comicPicture.nativeWidth
        height: comicPicture.nativeHeight
        anchors.fill: parent

        Flickable {
            id: viewContainer
            anchors.fill:parent

            contentWidth: comicPicture.nativeWidth
            contentHeight: comicPicture.nativeHeight

            QImageItem {
                id: comicPicture
                anchors { 
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                }
                smooth: true
                fillMode: QImageItem.PreserveAspectFit
 
                MouseArea {
                    id: dialogMouseArea
                    anchors.fill: comicPicture
                    onClicked: {
                        fullDialog.close();
                    }
                }
            }
        }
    }
        
    function open()
    {
        var pos = fullDialog.popupPosition(null, Qt.AlignCenter)
        fullDialog.x = pos.x
        fullDialog.y = pos.y

        fullDialog.visible = true
        fullDialog.activateWindow()
    }
    
    function close() {
        fullDialog.visible = false
    }
}