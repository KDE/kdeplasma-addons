/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami // For Action and Units

import org.kde.plasma.wallpapers.potd

Column {
    id: wallpaperPreview

    spacing: 0

    required property PotdBackend backend
    required property size screenSize

    /**
    * The background color of the preview area when the image is loaded
    */
    property alias backgroundColor: delegate.backgroundColor

    /**
    * The shadow height needs to be considered in the padding.
    */
    property alias shadowOffset: delegate.shadowOffset

    /**
    * The fill mode used for the image
    */
    property int fillMode

    // Wallpaper preview (including save button)
    WallpaperDelegate {
        id: delegate

        width: Math.round(wallpaperPreview.screenSize.width / 10 + Kirigami.Units.smallSpacing * 2) * Screen.devicePixelRatio
        height: Math.round(wallpaperPreview.screenSize.height / 10 + Kirigami.Units.smallSpacing * 2) * Screen.devicePixelRatio

        localUrl: wallpaperPreview.backend.localUrl
        infoUrl: wallpaperPreview.backend.infoUrl
        title: wallpaperPreview.backend.title
        author: wallpaperPreview.backend.author

        fillMode: wallpaperPreview.fillMode
        thumbnailAvailable: !delegate.isNull
        thumbnailLoading: wallpaperPreview.backend.loading

        actions: [
            Kirigami.Action {
                icon.name: "document-save"
                enabled: wallpaperPreview.backend.localUrl.length > 0
                visible: enabled
                tooltip: i18ndc("plasma_wallpaper_org.kde.potd", "@action:inmenu wallpaper preview menu", "Save Image as…")
                onTriggered: wallpaperPreview.backend.saveImage()

                Accessible.description: i18ndc("plasma_wallpaper_org.kde.potd", "@info:whatsthis for a button and a menu item", "Save today's picture to local disk")
            },
            Kirigami.Action {
                icon.name: "internet-services"
                enabled: wallpaperPreview.backend.infoUrl.toString().length > 0
                visible: false
                tooltip: i18ndc("plasma_wallpaper_org.kde.potd", "@action:inmenu wallpaper preview menu, will open the website of the wallpaper", "Open Link in Browser…")
                onTriggered: Qt.openUrlExternally(wallpaperPreview.backend.infoUrl)

                Accessible.description: i18ndc("plasma_wallpaper_org.kde.potd", "@info:whatsthis for a menu item", "Open the website of today's picture in the default browser")
            }
        ]
    }
}
