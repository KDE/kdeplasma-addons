/*
 * Rewrite of the KDE4-Plasma Binary Clock for KF5/Plasma/QML
 *
 * SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
 * SPDX-FileCopyrightText: 2018 Piotr Kąkol <piotrkakol@protonmail.com>
 *
 * Original code (KDE4):
 * SPDX-FileCopyrightText: 2007 Riccardo Iaconelli <riccardo@kde.org>
 * SPDX-FileCopyrightText: 2007 Davide Bettio <davide.bettio@kdemail.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0

Item {
    property int val
    property int bit

    /* both active and inactive squares */
    Rectangle {
        width:   main.dotSize
        height:  width
        color:   (val & bit) ? main.onColor : main.offColor
        x:       0
        y:       0
        visible: main.showOffLeds || color != main.offColor
    }

    /* bottom border for each square */
    Rectangle {
        visible: main.showGrid
        x:       0
        y:       main.dotSize
        width:   main.dotSize
        height:  units.smallSpacing
        color:   main.gridColor
    }

    /* right border for each square */
    Rectangle {
        visible: main.showGrid
        x:       main.dotSize
        y:       0
        width:   units.smallSpacing
        height:  main.dotSize+units.smallSpacing
        color:   main.gridColor
    }
}
