/*
 *   SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
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
    // Intentionally not using a standard duration value as this is an animated
    // wallpaper so disabling animations doesn't make sense, and the duration
    // shouldn't be scaled with the user's preferences because this is tuned to
    // create a specific visual effect.
    Behavior on color {
        SequentialAnimation {
            ColorAnimation {
                duration: 1000
                easing.type: Easing.InQuad
            }

            ScriptAction {
                script: wallpaper.repaintNeeded()
            }
        }
    }
}
