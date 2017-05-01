/*
 * Rewrite of the KDE4-Plasma Binary Clock for KF5/Plasma/QML
 *
 * Copyright 2014 Joseph Wenninger <jowenn@kde.org>
 *
 * Original code (KDE4):
 * Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>               *
 * Copyright 2007 by Davide Bettio <davide.bettio@kdemail.net>
 *
 * Based on FuzzyClock.qml:
 * Copyright 2013 Heena Mahour <heena393@gmail.com>
 * Copyright 2013 Sebastian Kügler <sebas@kde.org>
 * Copyright 2013 Martin Klapetek <mklapetek@kde.org>
 * Copyright 2014 David Edmundson <davidedmundson@kde.org>
 * Copyright 2014 Kai Uwe Broulik <kde@privat.broulik.de>
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
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as Components

Item {
    id: main

    property real w1: (main.height-5*units.smallSpacing)*dots/4

    Layout.minimumWidth: w1 < 20 ? 20 : w1
    Layout.maximumWidth: Infinity
    Layout.preferredWidth: Layout.minimumWidth

    Layout.minimumHeight: 16+(units.smallSpacing * 5)
   // Layout.maximumHeight: vertical ? Layout.minimumHeight : Infinity
    //Layout.preferredHeight: Layout.minimumHeight


    property int formFactor: plasmoid.formFactor
    property int timePixelSize: theme.defaultFont.pixelSize

    property bool constrained: formFactor == PlasmaCore.Types.Vertical || formFactor == PlasmaCore.Types.Horizontal

    property bool showSeconds: root.showSeconds

    property int hours:root.hours
    property int minutes:root.minutes
    property int seconds:root.seconds

    property bool showOffLeds: plasmoid.configuration.showOffLeds
    property bool showGrid: plasmoid.configuration.showGrid

    property int dots: showSeconds? 6:4

    property color onColor: plasmoid.configuration.useCustomColorForActive?plasmoid.configuration.customColorForActive: theme.textColor
    property color offColor: plasmoid.configuration.useCustomColorForInactive?plasmoid.configuration.customColorForInactive:Qt.rgba(onColor.r,onColor.g,onColor.b,0.4)
    property color gridColor: plasmoid.configuration.useCustomColorForGrid?plasmoid.configuration.customColorForGrid:Qt.rgba(onColor.r,onColor.g,onColor.b,0.6)

    property real dotSize: Math.min((height-5*units.smallSpacing)/4,(width-(dots+1)*units.smallSpacing)/dots)
    property real displayTop: (height - 4*dotSize-3*units.smallSpacing) /2
    property real displayLeft: (width - dots*dotSize-(dots-1)*units.smallSpacing) /2

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: plasmoid.expanded = !plasmoid.expanded
    }


    /*hours*/
    DotColumn {
        x:displayLeft
        y:displayTop
        val:hours
        startbit:4
    }
    DotColumn {
        x:displayLeft+(dotSize+units.smallSpacing)
        y:displayTop
        val:hours
        startbit:0
    }


    /*minutes*/
    DotColumn {
        x:displayLeft+(dotSize+units.smallSpacing)*2
        y:displayTop
        val:minutes
        startbit:4
    }
    DotColumn {
        x:displayLeft+(dotSize+units.smallSpacing)*3
        y:displayTop
        val:minutes
        startbit:0
    }

    /* seconds */
    DotColumn {
        x:displayLeft+(dotSize+units.smallSpacing)*4
        y:displayTop
        val:seconds
        startbit:4
        visible:showSeconds
    }

    DotColumn {
        x:displayLeft+(dotSize+units.smallSpacing)*5
        y:displayTop
        val:seconds
        startbit:0
        visible:showSeconds
    }

    Rectangle {
        x:displayLeft-units.smallSpacing
        y:displayTop-units.smallSpacing
        width:dots*(dotSize+units.smallSpacing)+units.smallSpacing
        height:units.smallSpacing
        visible:showGrid
        color:gridColor
    }

    Rectangle {
        x:displayLeft-units.smallSpacing
        y:displayTop
        width:units.smallSpacing
        height:4*(dotSize+units.smallSpacing)
        visible:showGrid
        color:gridColor
    }

}
