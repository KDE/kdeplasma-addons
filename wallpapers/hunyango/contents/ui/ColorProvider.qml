/*
 *   SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma Singleton

import QtQuick

Timer {
    property color color: _randomColor()

    function _randomColor() {
        return Qt.hsla(Math.random(), 1, 0.5, 1)
    }

    function updateColor(restartTimer = true) {
        color = _randomColor()

        if (restartTimer) {
            restart()
        }
    }

    running: true
    repeat: true
    interval: 30000
    onTriggered: updateColor(false)
}
