/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Window 2.0 // for Screen

import org.kde.kirigami 2.12 as Kirigami // For Action and Units

import org.kde.plasma.wallpapers.potd 1.0

Column {
    id: wallpaperPreview

    spacing: 0

    /**
     * The background color of the preview area when the image is loaded
     */
    property alias backgroundColor: delegate.backgroundColor

    // Wallpaper preview (including save button)
    WallpaperDelegate {
        id: delegate

        width: Math.round(Screen.width / 10 + Kirigami.Units.smallSpacing * 2)
        height: Math.round(Screen.height / 10 + Kirigami.Units.smallSpacing * 2)

        image: PotdProviderModelInstance.image
        localUrl: PotdProviderModelInstance.localUrl
        infoUrl: PotdProviderModelInstance.infoUrl
        title: PotdProviderModelInstance.title
        author: PotdProviderModelInstance.author

        thumbnailAvailable: !delegate.isNull
        thumbnailLoading: PotdProviderModelInstance.loading

        actions: [
            Kirigami.Action {
                icon.name: "document-save"
                enabled: PotdProviderModelInstance.localUrl.length > 0
                visible: enabled
                tooltip: i18nc("@action:inmenu wallpaper preview menu", "Save Image as…")
                onTriggered: PotdProviderModelInstance.saveImage()

                Accessible.description: i18nc("@info:whatsthis for a button and a menu item", "Save today's picture to local disk")
            },
            Kirigami.Action {
                icon.name: "internet-services"
                enabled: PotdProviderModelInstance.infoUrl.toString().length > 0
                visible: false
                tooltip: i18nc("@action:inmenu wallpaper preview menu, will open the website of the wallpaper", "Open Link in Browser…")
                onTriggered: Qt.openUrlExternally(PotdProviderModelInstance.infoUrl)

                Accessible.description: i18nc("@info:whatsthis for a menu item", "Open the website of today's picture in the default browser")
            }
        ]
    }

    Item {
        height: delegate.shadowOffset
    }
}
