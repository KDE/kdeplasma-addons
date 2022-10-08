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
 * Based on FuzzyClock.qml:
 * SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
 * SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
 * SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>
 * SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: main

    readonly property real w1: (main.height-5*PlasmaCore.Units.smallSpacing)*dots/4

    Layout.minimumWidth: w1 < 20 ? 20 : w1
    Layout.maximumWidth: Infinity
    Layout.preferredWidth: Layout.minimumWidth

    Layout.minimumHeight: 16+(PlasmaCore.Units.smallSpacing*5)
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

    readonly property color onColor: plasmoid.configuration.useCustomColorForActive ? plasmoid.configuration.customColorForActive : PlasmaCore.Theme.textColor
    readonly property color offColor: plasmoid.configuration.useCustomColorForInactive ? plasmoid.configuration.customColorForInactive : Qt.rgba(onColor.r, onColor.g, onColor.b, 0.4)
    readonly property color gridColor: plasmoid.configuration.useCustomColorForGrid ? plasmoid.configuration.customColorForGrid : Qt.rgba(onColor.r, onColor.g, onColor.b, 0.6)

    readonly property int dotSize: Math.min((height-5*PlasmaCore.Units.smallSpacing)/4, (width-(dots+1)*PlasmaCore.Units.smallSpacing)/dots)
    readonly property real displayTop: (height - 4*dotSize-3*PlasmaCore.Units.smallSpacing)/2
    readonly property real displayLeft: (width - dots*dotSize-(dots-1)*PlasmaCore.Units.smallSpacing)/2

    property bool wasExpanded: false

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onPressed: wasExpanded = plasmoid.expanded
        onClicked: plasmoid.expanded = !wasExpanded
    }

    /* hours */
    DotColumn {
        x:   displayLeft
        y:   displayTop
        val: hours/base
    }
    DotColumn {
        x:   displayLeft+(dotSize+PlasmaCore.Units.smallSpacing)
        y:   displayTop
        val: hours%base
    }

    /* minutes */
    DotColumn {
        x:   displayLeft+(dotSize+PlasmaCore.Units.smallSpacing)*2
        y:   displayTop
        val: minutes/base
    }
    DotColumn {
        x:   displayLeft+(dotSize+PlasmaCore.Units.smallSpacing)*3
        y:   displayTop
        val: minutes%base
    }

    /* seconds */
    DotColumn {
        x:       displayLeft+(dotSize+PlasmaCore.Units.smallSpacing)*4
        y:       displayTop
        val:     seconds/base
        visible: showSeconds
    }
    DotColumn {
        x:       displayLeft+(dotSize+PlasmaCore.Units.smallSpacing)*5
        y:       displayTop
        val:     seconds%base
        visible: showSeconds
    }

    /* upper grid border */
    Rectangle {
        x:       displayLeft-PlasmaCore.Units.smallSpacing
        y:       displayTop-PlasmaCore.Units.smallSpacing
        width:   dots*(dotSize+PlasmaCore.Units.smallSpacing)+PlasmaCore.Units.smallSpacing
        height:  PlasmaCore.Units.smallSpacing
        visible: showGrid
        color:   gridColor
    }

    /* left grid border */
    Rectangle {
        x:       displayLeft-PlasmaCore.Units.smallSpacing
        y:       displayTop
        width:   PlasmaCore.Units.smallSpacing
        height:  4*(dotSize+PlasmaCore.Units.smallSpacing)
        visible: showGrid
        color:   gridColor
    }
}
