/*
 * Rewrite of the KDE4-Plasma Binary Clock for KF5/Plasma/QML
 *
 * Copyright 2014 Joseph Wenninger <jowenn@kde.org>
 *
 * Original code (KDE4):
 * Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>               *
 * Copyright 2007 by Davide Bettio <davide.bettio@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0

Item {
    property int val
    property int bit

    Rectangle {
        width: main.dotSize
        height: width
        color: (val & bit) ? main.onColor:main.offColor
        x:0
        y:0
        visible:main.showOffLeds || color!=main.offColor
    }
    Rectangle {
        visible:main.showGrid
        x:0
        y:main.dotSize
        width:main.dotSize
        height:units.smallSpacing
        color:main.gridColor;
    }
    Rectangle {
        visible:main.showGrid
        x:main.dotSize
        y:0
        width:units.smallSpacing
        height:main.dotSize+units.smallSpacing
        color:main.gridColor
    }
}