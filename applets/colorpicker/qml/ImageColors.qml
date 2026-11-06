/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami

Kirigami.ImageColors {
    id: imageColors

    required property LoadingIndicator loadingIndicator
    signal addToHistoryRequested(string color)

    onPaletteChanged: {
        if (imageColors.palette.length > 0) {
            imageColors.addToHistoryRequested(imageColors.average);
        }
        loadingIndicator.jobDone();
        imageColors.destroy();
    }
}
