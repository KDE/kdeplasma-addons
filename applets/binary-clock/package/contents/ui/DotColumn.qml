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
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    property int val
    Dot {
        x:   0
        y:   0
        val: parent.val
        bit: 8
    }
    Dot {
        x:   0
        y:   (dotSize+PlasmaCore.Units.smallSpacing)
        val: parent.val
        bit: 4
    }
    Dot {
        x:   0
        y:   2*(dotSize+PlasmaCore.Units.smallSpacing)
        val: parent.val
        bit: 2
    }
    Dot {
        x:   0
        y:   3*(dotSize+PlasmaCore.Units.smallSpacing)
        val: parent.val
        bit: 1
    }
}
