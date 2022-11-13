/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.components 2.0 as PlasmaComponents // for ModelContextMenu

/**
 * Context menu to copy colors in different formats.
 */
PlasmaComponents.ModelContextMenu {
    id: formattingMenu

    required property QtObject picker
    required property Item colorLabel
    required property Item copyIndicatorLabel
    required property Timer colorLabelRestoreTimer

    onClicked: {
        picker.copyToClipboard(model.text)
        colorLabel.visible = false;
        copyIndicatorLabel.visible = true;
        colorLabelRestoreTimer.start()
    }

    onStatusChanged: {
        if (status == PlasmaComponents.DialogStatus.Closed) {
            formattingMenu.destroy();
        }
    }
}
