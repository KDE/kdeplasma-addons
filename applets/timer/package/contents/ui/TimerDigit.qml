/*
 *   SPDX-FileCopyrightText: 2008, 2014 Davide Bettio <davide.bettio@kdemail.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrolsaddons 2.0 as QtExtra

PlasmaCore.SvgItem {
        property int meaning;
        property int num;
        property string suffix;

        width: digitW;
        height: digitH;
        svg: timerSvg
        elementId: num + suffix;

        MouseArea {
            anchors.fill: parent;
            onWheel: {
                if (!root.running) {
                    if (wheel.angleDelta.y > 0) {
                        if (root.seconds + meaning < 360000) {
                            root.seconds += meaning;
                        }
                        main.digitChanged();

                    }else if (wheel.angleDelta.y < 0) {
                        if (root.seconds - meaning >= 0) {
                            root.seconds -= meaning;
                        }
                        main.digitChanged();
                    }
                }
            }
        }
}
