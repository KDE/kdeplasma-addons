/*
 *   SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.SvgItem {
    id: root
    x: parent.width - width
    width: naturalSize.width * (parent.height/naturalSize.height)
    height: parent.height
    svg: wallpaperSvg

    signal changeTriggered

    function change() {changeAnimation.running = true}

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
