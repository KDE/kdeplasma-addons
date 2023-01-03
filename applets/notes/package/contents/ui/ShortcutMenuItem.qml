/*
 *   SPDX-FileCopyrightText: 2019 Luca Carlon <carlon.luca@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

MenuItem {
    property alias _sequence: shortcutElement.sequence
    property alias _text: actionElement.text
    property alias _enabled: actionElement.enabled
    property alias _iconName: actionElement.icon.name

    Shortcut {
        id: shortcutElement
        enabled: false
    }

    action: Action {
        id: actionElement
        shortcut: shortcutElement.sequence
    }
}
