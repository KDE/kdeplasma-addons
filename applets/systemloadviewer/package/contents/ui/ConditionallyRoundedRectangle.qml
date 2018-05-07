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

Rectangle {
    property Rectangle borderRectangle

    readonly property real borderHeight: borderRectangle.height

    radius: borderRectangle.radius

    anchors.horizontalCenter: borderRectangle.horizontalCenter

    // Rectangles without rounded corners on the top and bottom if needed
    Rectangle {
        width: parent.width
        color: parent.color
        height: Math.min(parent.radius,
                         parent.height,
                         parent.borderHeight - parent.y - parent.radius / 2)
        visible: (parent.y - parent.radius > 0) && (
            parent.borderHeight - parent.y > parent.radius)
        anchors {
            top: parent.top
            left: parent.left
        }
    }

    Rectangle {
        width: parent.width
        color: parent.color
        height: Math.min(parent.radius, parent.height)
        visible: parent.y + parent.height < parent.borderHeight - parent.radius
        anchors {
            bottom: parent.bottom
            left: parent.left
        }
    }

    onHeightChanged: {
        width = borderRectangle.width - borderRectangle.border.width * 2
        if (borderHeight - y < radius) {
            width = width - (radius * 2) + (borderHeight-y)*2
        }
    }
}
