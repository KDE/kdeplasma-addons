/*
 * Rewrite of the KDE4-Plasma Binary Clock for KF5/Plasma/QML
 *
 * SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
 * SPDX-FileCopyrightText: 2018 Piotr Kąkol <piotrkakol@protonmail.com>
 * SPDX-FileCopyrightText: 2023 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
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
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid

Item {
    id: main

    readonly property real w1: (main.height-5*Kirigami.Units.smallSpacing)*dots/4

    Layout.minimumWidth: w1 < 20 ? 20 : w1
    Layout.maximumWidth: Infinity
    Layout.preferredWidth: Layout.minimumWidth

    Layout.minimumHeight: 16+(Kirigami.Units.smallSpacing*5)
    //Layout.maximumHeight: vertical ? Layout.minimumHeight : Infinity
    //Layout.preferredHeight: Layout.minimumHeight

    readonly property int formFactor: Plasmoid.formFactor

    readonly property bool constrained: formFactor == PlasmaCore.Types.Vertical || formFactor == PlasmaCore.Types.Horizontal

    required property int hours
    required property int minutes
    required property int seconds

    readonly property int base: 10

    readonly property bool showOffLeds: Plasmoid.configuration.showOffLeds

    readonly property int dots: Plasmoid.configuration.showSeconds ? 6 : 4

    readonly property color onColor: Plasmoid.configuration.useCustomColorForActive ? Plasmoid.configuration.customColorForActive : Kirigami.Theme.textColor
    readonly property color offColor: Plasmoid.configuration.useCustomColorForInactive ? Plasmoid.configuration.customColorForInactive : Qt.rgba(onColor.r, onColor.g, onColor.b, 0.2)

    readonly property int dotSize: Math.min((height-5*Kirigami.Units.smallSpacing)/4, (width-(dots+1)*Kirigami.Units.smallSpacing)/dots)

    readonly property alias mouseArea: mouseArea

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }

    GridLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        columns: Plasmoid.configuration.showSeconds ? 6 : 4
        Repeater {
            model: [8, 4, 2, 1]
            Repeater {
                id: bitRepeater
                required property var modelData
                model: [main.hours/main.base, main.hours%main.base, main.minutes/main.base, main.minutes%main.base, main.seconds/main.base, main.seconds%main.base]
                property var bit: modelData
                Rectangle {
                    required property var modelData
                    required property int index
                    property var timeVal: modelData
                    visible: main.dotSize >= 0 && (main.Plasmoid.configuration.showSeconds || index < 4)
                    width: main.dotSize
                    height: width
                    radius: width/2
                    color: (timeVal & bitRepeater.bit) ? main.onColor : (main.showOffLeds ? main.offColor : "transparent")
                }
            }
        }
    }
}
