/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3

/**
 * Visible when an image is being processed by \Kirigami.ImageColors
 */
PlasmaComponents3.BusyIndicator {
    id: loadingIndicator
    anchors.fill: parent

    property int jobRemaining: 0

    signal jobDone

    onJobDone: {
        jobRemaining -= 1;
        if (!jobRemaining) {
            animator.start();
        }
    }

    onJobRemainingChanged: selfDestructionTimer.restart();

    OpacityAnimator {
        id: animator
        easing.type: Easing.InQuad
        from: 1
        to: 0
        target: loadingIndicator
        duration: PlasmaCore.Units.longDuration

        onStopped: selfDestructionTimer.triggered();
    }

    Timer {
        id: selfDestructionTimer
        interval: 60000
        running: true

        onTriggered: {
            loadingIndicator.parent.loadingIndicator = null;
            loadingIndicator.destroy();
        }
    }
}
