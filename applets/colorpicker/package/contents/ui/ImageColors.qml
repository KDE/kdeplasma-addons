/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.kirigami 2.20 as Kirigami

Image {
    id: component

    width: 128
    height: width
    visible: false

    asynchronous: true
    cache: false
    autoTransform: true

    onStatusChanged: {
        switch (status) {
        case Image.Loading:
            break;
        case Image.Ready:
            imageColors.source = component;
            break;
        default:
            component.destroy();
        }
    }

    Timer {
        interval: 5000
        running: true

        onTriggered: component.destroy()
    }

    Kirigami.ImageColors {
        id: imageColors

        onPaletteChanged: {
            if (imageColors.palette.length > 0) {
                root.addColorToHistory(imageColors.average);
            }
            component.destroy();
        }
    }
}

