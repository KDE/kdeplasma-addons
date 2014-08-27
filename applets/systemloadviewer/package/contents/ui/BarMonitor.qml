/*
 * Copyright (C) 2014 Martin Yrjölä <martin.yrjola@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1

Item {
    id: barMonitor

    property var colors
    property var proportions

    Layout.fillHeight: true
    Layout.fillWidth: true

    Rectangle {
        id: barBorder
        anchors.fill: parent
        color: "transparent"
        radius: 3
        opacity: .4
        border {
            color: theme.textColor
            width: 1
        }
    }

    Repeater {
        id: barRepeater
        model: proportions.length
        ConditionallyRoundedRectangle {
            color: barMonitor.colors[index]
            height: (barBorder.height - barBorder.border.width) * barMonitor.proportions[index]
            borderRectangle: barBorder
            // Bars should always be below border
            z: -1
            anchors {
                bottom: index == 0 ? barBorder.bottom : barRepeater.itemAt(index-1).top
                bottomMargin: index == 0 ? barBorder.border.width : 0
            }
        }
    }
}
