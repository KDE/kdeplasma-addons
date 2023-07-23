/*
 *   SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.plasmoid 2.0

WallpaperItem {
    id: root

    contextualActions: [
        PlasmaCore.Action {
            text: i18nd("plasma_wallpaper_org.kde.hunyango", "Randomize Wallpaper Color")
            icon.name: "color-profile"
            onTriggered: {
                updateTimer.restart()
                rectangle.updateColor()
            }
        }
    ]

    Rectangle {
        id: rectangle
        anchors.fill: parent

        color: randomColor()

        function randomColor() {
            return Qt.hsla(Math.random(), 1, 0.5, 1)
        }

        function updateColor() {
            color = randomColor()
        }

        KSvg.SvgItem {
            anchors.fill: parent
            smooth: true
            svg: KSvg.Svg {
                id: wallpaperSvg
                //FIXME: Svg doesn't support relative paths
                imagePath: String(Qt.resolvedUrl("wallpaper.svgz")).substring(7)
            }
        }

        Timer {
            id: updateTimer

            property int lastLayer: 0
            property variant layers: [centerLayer0, centerLayer1, leftLayer0, leftLayer1, bottomLayer0, rightLayer0, rightLayer1]

            running: true
            repeat: true
            interval: 30000
            onTriggered: rectangle.updateColor()
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
                    script: root.repaintNeeded()
                }
            }
        }
    }
}
