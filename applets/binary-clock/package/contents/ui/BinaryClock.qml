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

    readonly property real w1: (main.height-5*units.smallSpacing)*dots/4

    Layout.minimumWidth: w1 < 20 ? 20 : w1
    Layout.maximumWidth: Infinity
    Layout.preferredWidth: Layout.minimumWidth

    Layout.minimumHeight: 16+(units.smallSpacing*5)
    //Layout.maximumHeight: vertical ? Layout.minimumHeight : Infinity
    //Layout.preferredHeight: Layout.minimumHeight

    readonly property int formFactor: plasmoid.formFactor

    readonly property bool constrained: formFactor == PlasmaCore.Types.Vertical || formFactor == PlasmaCore.Types.Horizontal

    readonly property bool showSeconds: root.showSeconds

    readonly property int hours: root.hours
    readonly property int minutes: root.minutes
    readonly property int seconds: root.seconds

    readonly property bool showOffLeds: plasmoid.configuration.showOffLeds
    readonly property bool showGrid: plasmoid.configuration.showGrid

    readonly property int base: plasmoid.configuration.showBcdFormat ? 10 : 16

    readonly property int dots: showSeconds ? 6 : 4

    readonly property color onColor: plasmoid.configuration.useCustomColorForActive ? plasmoid.configuration.customColorForActive : theme.textColor
    readonly property color offColor: plasmoid.configuration.useCustomColorForInactive ? plasmoid.configuration.customColorForInactive : Qt.rgba(onColor.r, onColor.g, onColor.b, 0.4)
    readonly property color gridColor: plasmoid.configuration.useCustomColorForGrid ? plasmoid.configuration.customColorForGrid : Qt.rgba(onColor.r, onColor.g, onColor.b, 0.6)

    readonly property int dotSize: Math.min((height-5*units.smallSpacing)/4, (width-(dots+1)*units.smallSpacing)/dots)
    readonly property real displayTop: (height - 4*dotSize-3*units.smallSpacing)/2
    readonly property real displayLeft: (width - dots*dotSize-(dots-1)*units.smallSpacing)/2

    /* displaying calendar after clicking binary clock applet */
    MouseArea {
        id:           mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked:    plasmoid.expanded = !plasmoid.expanded
    }

    /* hours */
    DotColumn {
        x:   displayLeft
        y:   displayTop
        val: hours/base
    }
    DotColumn {
        x:   displayLeft+(dotSize+units.smallSpacing)
        y:   displayTop
        val: hours%base
    }

    /* minutes */
    DotColumn {
        x:   displayLeft+(dotSize+units.smallSpacing)*2
        y:   displayTop
        val: minutes/base
    }
    DotColumn {
        x:   displayLeft+(dotSize+units.smallSpacing)*3
        y:   displayTop
        val: minutes%base
    }

    /* seconds */
    DotColumn {
        x:       displayLeft+(dotSize+units.smallSpacing)*4
        y:       displayTop
        val:     seconds/base
        visible: showSeconds
    }
    DotColumn {
        x:       displayLeft+(dotSize+units.smallSpacing)*5
        y:       displayTop
        val:     seconds%base
        visible: showSeconds
    }

    /* upper grid border */
    Rectangle {
        x:       displayLeft-units.smallSpacing
        y:       displayTop-units.smallSpacing
        width:   dots*(dotSize+units.smallSpacing)+units.smallSpacing
        height:  units.smallSpacing
        visible: showGrid
        color:   gridColor
    }

    /* left grid border */
    Rectangle {
        x:       displayLeft-units.smallSpacing
        y:       displayTop
        width:   units.smallSpacing
        height:  4*(dotSize+units.smallSpacing)
        visible: showGrid
        color:   gridColor
    }
}
