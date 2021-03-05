/*
 *   SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.SvgItem {
    id: root
    height: naturalSize.height * (parent.width/naturalSize.width)
    width: parent.width
    y: parent.height - height
    svg: wallpaperSvg

    signal changeTriggered

    function change() {changeAnimation.running = true}

    // Intentionally not using a standard duration value for these, as this is
    // an animated wallpaper so disabling animations doesn't make sense, and the
    // duration shouldn't be scaled with the user's preferences because this is
    // tuned to create a specific visual effect.
    SequentialAnimation {
        id: changeAnimation
        NumberAnimation {
            targets: root
            properties: "opacity"
            to: 0
            duration: 1000
            easing.type: Easing.InOutCubic
        }

        ScriptAction { script: root.z = Math.floor(Math.random()*7) }

        NumberAnimation {
            targets: root
            properties: "opacity"
            to: Math.random()
            duration: 1000
            easing.type: Easing.InOutCubic
        }
    }
}
