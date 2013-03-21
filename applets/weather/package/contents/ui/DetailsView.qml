/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
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
import org.kde.qtextracomponents 0.1 as QtExtraComponents

WeatherListView {
    id: root

    roundedRows: false

    delegate: Item {
        anchors.fill: parent

        Item {
            anchors.centerIn: parent
            height: parent.height
            width: childrenRect.width

            QtExtraComponents.QPixmapItem {
                id: icon
                pixmap: svg.pixmap(rowData.icon)
                height: nativeHeight
                width: nativeWidth
                visible: rowData.icon.length > 0
            }

            Text {
                anchors {
                    left: icon.right
                    leftMargin: 2
                    verticalCenter: parent.verticalCenter
                }
                color: theme.textColor
                text: rowData.text
            }
        }
    }

    PlasmaCore.Svg {
        id: svg
        imagePath: "weather/wind-arrows"
    }
}
