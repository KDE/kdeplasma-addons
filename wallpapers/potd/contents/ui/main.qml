/*
 *   Copyright 2016 Weng Xuetian <wengxt@gmail.com>
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

import QtQuick 2.5
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrolsaddons 2.0

Rectangle {
    id: root

    readonly property string provider: wallpaper.configuration.Provider

    PlasmaCore.DataSource {
        id: engine
        engine: "potd"
        connectedSources: [provider]
    }

    Rectangle {
        id: backgroundColor
        anchors.fill: parent
        color: wallpaper.configuration.Color
        Behavior on color {
            ColorAnimation { duration: units.longDuration }
        }
    }

    QImageItem {
        anchors.fill: parent
        image: engine.data[provider]["Image"]
        fillMode: wallpaper.configuration.FillMode
    }
}
