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
import "Utils.js" as Utils

WeatherListView {
    id: root

    delegate: Row {
        anchors.fill: parent

        Repeater {
            id: rowRepeater

            model: rowData

            Loader {
                height: rowIndex == 1 ? parent.height + root.spacing : parent.height
                width: parent.width / rowRepeater.count
                sourceComponent: rowIndex == 1 ? iconDelegate : textDelegate
                onLoaded: {
                    if (rowIndex == 1) {
                        var values = modelData.split("|");
                        item.icon = values[0];
                        item.toolTip = values[1];
                    } else {
                        item.text = modelData;
                    }

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
            color: {
                if (!font.bold)
                    return theme.textColor

                var ntIndex = text.indexOf("nt");
                if (ntIndex != -1 || text.indexOf("nite") != -1) {
                    if (ntIndex != -1)
                        text = text.replace(" nt", "");

                    return Utils.setAlphaF(theme.textColor, 0.5);
                } else {
                    return theme.textColor
                }
            }
        }
    }

    Component {
        id: iconDelegate

        QtExtraComponents.QIconItem {
            property alias toolTip: iconToolTip.mainText
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -root.spacing/2

            PlasmaCore.ToolTip {
                id: iconToolTip
                target: parent
            }
        }
    }
}
