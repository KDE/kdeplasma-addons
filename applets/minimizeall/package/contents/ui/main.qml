/*
 *    Copyright 2015 Sebastian Kügler <sebas@kde.org>
 *    Copyright 2016 Anthony Fieroni <bvbfan@abv.bg>
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

import org.kde.plasma.private.minimizeall 1.0


Item {
    id: root

    Layout.minimumWidth: units.gridUnit
    Layout.minimumHeight: units.gridUnit

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    function activate() {
        if (!minimizeall.active) {
            minimizeall.minimizeAllWindows();
        } else {
            minimizeall.unminimizeAllWindows();
        }
    }

    Plasmoid.onActivated: activate()

    MinimizeAll {
        id: minimizeall
    }

    PlasmaCore.FrameSvgItem {
        id: expandedItem
        anchors.fill: parent
        imagePath: "widgets/tabbar"
        prefix: {
            var prefix;
            switch (plasmoid.location) {
                case PlasmaCore.Types.LeftEdge:
                    prefix = "west-active-tab";
                    break;
                case PlasmaCore.Types.TopEdge:
                    prefix = "north-active-tab";
                    break;
                case PlasmaCore.Types.RightEdge:
                    prefix = "east-active-tab";
                    break;
                default:
                    prefix = "south-active-tab";
            }
            if (!hasElementPrefix(prefix)) {
                prefix = "active-tab";
            }
            return prefix;
        }
        opacity: minimizeall.active ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: units.shortDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    PlasmaCore.IconItem {
        id:icon
        source: plasmoid.configuration.icon
        active: mouseArea.containsMouse
        anchors.fill: parent
    }

    PlasmaCore.ToolTipArea {
        anchors.fill: parent
        mainText : i18n("Minimize Windows")
        subText : i18n("Show the desktop by minimizing all windows")
        icon : plasmoid.configuration.icon

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: activate()
        }
    }
}
