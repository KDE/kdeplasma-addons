/*
 *    Copyright 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore


Item {
    id: root

    Plasmoid.onActivated: toggle()

    PlasmaCore.DataSource {
        id: keystateSource
        engine: "keystate"
        connectedSources: ["Caps Lock", "Num Lock"]
    }

    readonly property bool capsLock: keystateSource.data["Caps Lock"]["Locked"]
    readonly property bool numLock: keystateSource.data["Num Lock"]["Locked"]
    readonly property string capsLockMessage: capsLock ? i18n("Caps Lock is on") : i18n("Caps Lock is off")
    readonly property string numLockMessage: numLock ? i18n("Num Lock is on") : i18n("Num Lock is off")

    Plasmoid.icon: capsLock ? "input-caps-on" : "input-keyboard"
    Plasmoid.toolTipSubText: capsLockMessage + "\n" + numLockMessage
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.fullRepresentation: PlasmaCore.ToolTipArea {
        readonly property bool inPanel: (plasmoid.location === PlasmaCore.Types.TopEdge
            || plasmoid.location === PlasmaCore.Types.RightEdge
            || plasmoid.location === PlasmaCore.Types.BottomEdge
            || plasmoid.location === PlasmaCore.Types.LeftEdge)

        Layout.minimumWidth: units.iconSizes.small
        Layout.minimumHeight: Layout.minimumWidth

        Layout.maximumWidth: inPanel ? units.iconSizeHints.panel : -1
        Layout.maximumHeight: inPanel ? units.iconSizeHints.panel : -1

        icon: plasmoid.icon
        mainText: plasmoid.title
        subText: plasmoid.toolTipSubText

        PlasmaCore.IconItem {
            anchors.fill: parent
            source: plasmoid.icon
            active: parent.containsMouse || root.capsLock
        }
    }
}
