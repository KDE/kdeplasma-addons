/*
 *   SPDX-FileCopyrightText: 2019 Luca Carlon <carlon.luca@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2

QQC2.MenuItem {
    id: root

    property var _sequence: []
    property alias _text: actionElement.text
    property alias _enabled: actionElement.enabled
    property alias _iconName: actionElement.icon.name

    Shortcut {
        id: shortcutElement
        sequences: [root._sequence]
        enabled: false
    }

    action: QQC2.Action {
        id: actionElement
        shortcut: shortcutElement.sequences ? shortcutElement.sequences[0] : null
    }
}
