/*
 * SPDX-FileCopyrightText: 2014 Martin Yrjölä <martin.yrjola@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore

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

        readonly property int lineWidth: 1
        readonly property bool fill: true
        readonly property bool stroke: true
        readonly property real alpha: 1.0

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
            ctx.strokeStyle = PlasmaCore.ColorScope.textColor
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
