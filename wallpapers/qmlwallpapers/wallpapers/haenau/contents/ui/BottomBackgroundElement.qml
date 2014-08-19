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

PlasmaCore.SvgItem {
    id: root
    height: naturalSize.height * (parent.width/naturalSize.width)
    width: parent.width
    y: parent.height - height
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
