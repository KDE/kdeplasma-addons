/*
 * SPDX-FileCopyrightText: 2014 Martin Yrjölä <martin.yrjola@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
