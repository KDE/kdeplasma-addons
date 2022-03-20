/*
 *   SPDX-FileCopyrightText: 2016 Weng Xuetian <wengxt@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.5
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrolsaddons 2.0

import org.kde.plasma.wallpapers.potd 1.0

Rectangle {
    id: root

    PotdProviderModel {
        id: engine
        identifier: wallpaper.configuration.Provider
        // Needs to specify category for unsplash provider
        arguments: identifier === "unsplash" ? [wallpaper.configuration.Category] : []
        running: true
    }

    Rectangle {
        id: backgroundColor
        anchors.fill: parent
        color: wallpaper.configuration.Color
        Behavior on color {
            ColorAnimation { duration: PlasmaCore.Units.longDuration }
        }
    }

    QImageItem {
        anchors.fill: parent
        image: engine.image
        fillMode: wallpaper.configuration.FillMode
        smooth: true
    }
}
