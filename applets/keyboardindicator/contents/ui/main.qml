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

    readonly property QtObject source: PlasmaCore.DataSource {
        id: keystateSource
        engine: "keystate"
        connectedSources: [plasmoid.configuration.key]
    }

    function translate(identifier) {
        switch(identifier) {
            case "CapsLock": return i18n("Caps Lock")
            case "Num Lock": return i18n("Num Lock")
        }
        return identifier;
    }
    function icon(identifier) {
        switch(identifier) {
            case "CapsLock": return "input-caps-on"
        }
        return "emblem-locked";
    }

    readonly property bool isLocked: keystateSource.data[plasmoid.configuration.key].Locked
    readonly property string message: isLocked ? i18n("%1 is locked", translate(plasmoid.configuration.key)) : i18n("%1 is unlocked", translate(plasmoid.configuration.key))

    Plasmoid.icon: isLocked ? "input-caps-on" : ""
    Plasmoid.title: message
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.fullRepresentation: PlasmaCore.ToolTipArea {
        readonly property bool inPanel: (plasmoid.location === PlasmaCore.Types.TopEdge
            || plasmoid.location === PlasmaCore.Types.RightEdge
            || plasmoid.location === PlasmaCore.Types.BottomEdge
            || plasmoid.location === PlasmaCore.Types.LeftEdge)

        Layout.minimumWidth: isLocked ? units.iconSizes.small : 0
        Layout.minimumHeight: isLocked ? Layout.minimumWidth : 0

        Layout.maximumWidth: inPanel ? units.iconSizeHints.panel : -1
        Layout.maximumHeight: inPanel ? units.iconSizeHints.panel : -1

        icon: plasmoid.icon || "input-keyboard"
        mainText: plasmoid.title
        subText: plasmoid.toolTipSubText

        PlasmaCore.IconItem {
            anchors.fill: parent
            source: plasmoid.icon
            active: parent.containsMouse || root.isLocked
        }
    }
}
