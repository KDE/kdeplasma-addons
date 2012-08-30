/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: root

    property int value: 0
    property alias dotVisible: dot.visible

    implicitWidth: dummy.naturalSize.width
    implicitHeight: dummy.naturalSize.height

    QtObject {
        id: internal
        property real dotWidthRate: dot.naturalSize.width / dummy.naturalSize.width
        property real digitWidthRate: digit.naturalSize.width /
                                      (dummy.naturalSize.width - dot.naturalSize.width)
        property variant values: ["zero", "one", "two", "three", "four",
                                  "five", "six", "seven", "eight", "nine"]
    }

    PlasmaCore.Svg {
        id: digitSvg
        imagePath: "weatherstation/lcd_digits"
    }

    // Used just to get the size hints of digit + dot
    PlasmaCore.SvgItem { id: dummy; svg: digitSvg; visible: false }

    Item {
        id: item
        height: parent.height
        width: parent.width * (1 - internal.dotWidthRate)

        PlasmaCore.SvgItem {
            id: digit
            anchors.right: parent.right
            width: parent.width * internal.digitWidthRate
            height: parent.height
            svg: digitSvg
            elementId: internal.values[root.value]
        }
    }

    PlasmaCore.SvgItem {
        id: dot
        anchors{
            right: parent.right
            bottom: parent.bottom
        }
        width: parent.width * internal.dotWidthRate
        height: dot.width
        visible: false
        svg: digitSvg
        elementId: "dot"
    }
}
