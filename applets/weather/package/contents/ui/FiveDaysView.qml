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
import org.kde.qtextracomponents 0.1 as QtExtraComponents

WeatherListView {
    id: root

    spacing: 18
    rows: 4

    property variant model: [["Fri", "Sat", "Sun"],
            ["weather-many-clouds", "weather-clouds", "weather-clear"],
            ["27°C", "30°C", "30°C"],
            ["21°C", "20°C", "21°C"]]

    delegate: Row {
        property int rowIndex

        anchors.fill: parent

        Repeater {
            id: rowRepeater

            model: root.model[rowIndex]

            Loader {
                height: rowIndex == 1 ? parent.height + 15 : parent.height
                width: parent.width / rowRepeater.count
                sourceComponent: rowIndex == 1 ? iconDelegate : textDelegate
                onLoaded: {
                    if (rowIndex == 1)
                        item.icon = modelData;
                    else
                        item.text = modelData;

                    if (rowIndex == 0)
                        item.font.bold = true;
                }
            }
        }
    }

    Component {
        id: textDelegate

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    Component {
        id: iconDelegate

        QtExtraComponents.QIconItem {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -8
        }
    }
}
