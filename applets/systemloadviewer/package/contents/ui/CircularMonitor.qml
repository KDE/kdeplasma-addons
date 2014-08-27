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

import QtQuick 2.0
import QtQuick.Layouts 1.1

Item {
    property alias colors: canvas.colors
    property alias proportions: canvas.proportions

    Layout.fillHeight: true
    Layout.fillWidth: true

    onProportionsChanged: {
        canvas.requestPaint()
    }

    Canvas {
        id: canvas

        property int lineWidth: 1
        property bool fill: true
        property bool stroke: true
        property real alpha: 1.0

        property var proportions
        property var colors

        // This fixes edge bleeding
        readonly property double filler: 0.01

        width: parent.width
        height: Math.min(parent.height, parent.width)
        antialiasing: true

        onPaint: {
            var ctx = getContext("2d");
            ctx.save();
            ctx.clearRect(0,0,canvas.width, canvas.height);
            ctx.strokeStyle = canvas.strokeStyle;
            ctx.globalAlpha = canvas.alpha

            // Start from 6 o'clock
            var currentRadian = Math.PI/2

            // Draw the sectors
            for (var i = 0; i < proportions.length; i++) {
                var radians = proportions[i] * 2 * Math.PI
                ctx.fillStyle = colors[i]
                ctx.beginPath();
                ctx.arc(width/2, height/2, height/2.1, currentRadian, currentRadian + radians + filler, false)
                ctx.arc(width/2, height/2, height/4, currentRadian + radians + filler, currentRadian, true)
                currentRadian += radians - filler
                ctx.closePath();
                ctx.fill();
            }

            // Draw border
            ctx.fillStyle = "transparent"
            ctx.strokeStyle = theme.textColor
            ctx.lineWidth = canvas.lineWidth
            ctx.globalAlpha = .4
            ctx.beginPath();
            ctx.arc(width/2, height/2, height/2.1, Math.PI/2, 5*Math.PI/2, false)
            ctx.moveTo(width/2,height/2+height/4);
            ctx.arc(width/2, height/2, height/4, Math.PI/2, 5*Math.PI/2, false)
            ctx.fillRule = Qt.OddEvenFill
            ctx.fill();
            ctx.stroke();
            ctx.restore();
        }
    }
}
