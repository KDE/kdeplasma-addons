/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {
    id: root
    twinFormLayouts: parentLayout

    property string cfg_Image

    Button {
        Kirigami.FormData.label: i18ndc("plasma_wallpaper_org.kde.timed", "@label", "Tile:")
        icon.source: cfg_Image
        onClicked: fileDialog.open()
    }

    FileDialog {
        id: fileDialog
        currentFolder: {
            let defaultPaths = StandardPaths.standardLocations(StandardPaths.PicturesLocation);
            if (!defaultPaths.length) {
                defaultPaths = StandardPaths.standardLocations(StandardPaths.HomeLocation);
            }
            return defaultPaths[0];
        }
        fileMode: FileDialog.OpenFiles
        options: FileDialog.ReadOnly
        title: i18ndc("plasma_wallpaper_org.kde.tiled", "@title:window", "Open Image")
        onAccepted: cfg_Image = selectedFile;
    }
}
