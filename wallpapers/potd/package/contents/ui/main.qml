/*
 *   SPDX-FileCopyrightText: 2016 Weng Xuetian <wengxt@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.5
import QtQuick.Window 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrolsaddons 2.0

import org.kde.plasma.wallpapers.potd 1.0

Rectangle {
    id: root

    PotdProviderModel {
        id: engine
        identifier: wallpaper.configuration.Provider
        arguments: {
            if (identifier === "unsplash") {
                // Needs to specify category for unsplash provider
                return [wallpaper.configuration.Category];
            } else if (identifier === "bing") {
                // Bing supports 1366/1920/UHD resolutions
                return [Screen.width, Screen.height, Screen.devicePixelRatio];
            }
            return [];
        }
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
