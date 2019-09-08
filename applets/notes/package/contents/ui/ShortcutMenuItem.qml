/*
 *   Copyright 2019 Luca Carlon <carlon.luca@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
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

import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 2.12

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
        shortcut: shortcutElement.nativeText
    }
}