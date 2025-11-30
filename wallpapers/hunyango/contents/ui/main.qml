/*
 *   SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.plasma.core as PlasmaCore
import org.kde.ksvg as KSvg
import org.kde.plasma.plasmoid

WallpaperItem {
    id: root

    contextualActions: [
        PlasmaCore.Action {
            text: i18nd("plasma_wallpaper_org.kde.hunyango", "Randomize Wallpaper Color")
            icon.name: "color-profile"
            onTriggered: ColorProvider.updateColor(true)
        }
    ]

    Rectangle {
        id: rectangle
        anchors.fill: parent

        color: ColorProvider.color

        KSvg.SvgItem {
            // Enforce 16:9 aspect ratio and fill centered, rather than stretch the background
            anchors.centerIn: parent
            width: (parent.width / parent.height >= 16 / 9) ? parent.width : height * (16 / 9)
            height: (parent.width / parent.height >= 16 / 9) ? width * (9 / 16) : parent.height

            smooth: true

            //FIXME: Svg doesn't support relative paths
            imagePath: String(Qt.resolvedUrl("wallpaper.svgz")).substring(7)
        }
    }
}
