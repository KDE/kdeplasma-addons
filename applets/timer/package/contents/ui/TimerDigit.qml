/***************************************************************************
 *   Copyright 2008,2014 by Davide Bettio <davide.bettio@kdemail.net>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
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
