/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

/**
 * Context menu to copy colors in different formats.
 */
PlasmaExtras.ModelContextMenu {
    id: formattingMenu

    required property QtObject picker
    required property Item colorLabel
    required property Item copyIndicatorLabel
    required property Timer colorLabelRestoreTimer

    placement: PlasmaCore.Types.BottomPosedLeftAlignedPopup

    onClicked: {
        picker.copyToClipboard(model.text)
        colorLabel.visible = false;
        copyIndicatorLabel.visible = true;
        colorLabelRestoreTimer.start()
    }

    onStatusChanged: {
        if (status == PlasmaExtras.Menu.Closed) {
            formattingMenu.destroy();
        }
    }
}
