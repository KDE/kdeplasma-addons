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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore

PlasmaCore.SvgItem {
    id: root
    x: parent.width - width
    width: naturalSize.width * (parent.height/naturalSize.height)
    height: parent.height
    svg: wallpaperSvg

    signal slideTriggered

    function slide() {slideAnimation.running = true}

    SequentialAnimation {
        id: slideAnimation
        NumberAnimation {
            targets: root
            properties: "x"
            to: root.parent.width
            duration: 1000
            easing.type: Easing.InOutCubic
        }

        ScriptAction { script: root.z = Math.floor(Math.random()*7) }

        NumberAnimation {
            targets: root
            properties: "x"
            to: root.parent.width - width
            duration: 1000
            easing.type: Easing.InOutCubic
        }
    }
}
