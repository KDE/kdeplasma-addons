/*
 *   Copyright 2012 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Rectangle {
    id: root
    width: 800
    height: 480

    function randomColor() {
        root.color = Qt.hsla(Math.random(), 1,0.5,1)
    }

    Component.onCompleted: randomColor()
    MouseArea {
        anchors.fill: parent
        onClicked: randomColor()
    }

    PlasmaCore.SvgItem {
        anchors.fill: parent
        smooth: true
        svg: PlasmaCore.Svg {
            id: wallpaperSvg
            //FIXME: Svg doesn't support relative paths
            imagePath: Qt.resolvedUrl("wallpaper.svgz").substring(7)
        }
    }

    Timer {
        property int lastLayer: 0
        property variant layers: [centerLayer0, centerLayer1, leftLayer0, leftLayer1, bottomLayer0, rightLayer0, rightLayer1]
        running: true
        repeat: true
        interval: 30000
        onTriggered: randomColor()
    }
    Behavior on color { ColorAnimation { duration: 1000; easing.type: Easing.InQuad } }
}