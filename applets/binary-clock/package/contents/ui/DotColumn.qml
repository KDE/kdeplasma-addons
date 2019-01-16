/*
 * Rewrite of the KDE4-Plasma Binary Clock for KF5/Plasma/QML
 *
 * Copyright 2014 Joseph Wenninger <jowenn@kde.org>
 * Copyright 2018 Piotr Kąkol <piotrkakol@protonmail.com>
 *
 * Original code (KDE4):
 * Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>
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
    Dot {
        x:   0
        y:   0
        val: parent.val
        bit: 8
    }
    Dot {
        x:   0
        y:   (dotSize+units.smallSpacing)
        val: parent.val
        bit: 4
    }
    Dot {
        x:   0
        y:   2*(dotSize+units.smallSpacing)
        val: parent.val
        bit: 2
    }
    Dot {
        x:   0
        y:   3*(dotSize+units.smallSpacing)
        val: parent.val
        bit: 1
    }
}