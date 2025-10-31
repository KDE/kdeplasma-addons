/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import org.kde.plasma.plasmoid

WallpaperItem {
    id: root

    onOpenUrlRequested: (url) => {
        root.configuration.Image = url;
        root.configuration.writeConfig();
    }

    Rectangle {
        anchors.fill: parent
        color: "blue"
    }

    Image {
        anchors.fill: parent
        source: root.configuration.Image
        fillMode: Image.Tile
    }
}
