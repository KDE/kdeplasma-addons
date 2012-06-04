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
    id: display

    property string number
    property alias superscript: unitLabel.text
    property alias superscriptFont: unitLabel.font

    implicitWidth: row.width
    implicitHeight: dummy.implicitHeight

    QtObject {
        id: internal

        property bool decimal: display.number.indexOf(".") > 0
        property bool dotIndex: display.number.length - display.number.indexOf(".")
        property real resizeRate: display.height / dummy.implicitHeight

        function numberToDigits(numberStr) {
            digits = [];
            for(var i = 0; i < numberStr.length; i++) {
                if (numberStr[i] >= '0' && numberStr[i] <= '9')
                    digits.push(numberStr[i] - '0');
            }
            return digits;
        }
    }

    // Just to get the implicitHeight.
    LCDDigit { id: dummy; visible: false }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 2 * internal.resizeRate

        Repeater {
            model: internal.numberToDigits(display.number);

            LCDDigit {
                height: implicitHeight * internal.resizeRate
                width: implicitWidth * internal.resizeRate
                value: modelData
                dotVisible: internal.decimal && (index == internal.dotIndex)
            }
        }
    }

    Text {
        id: unitLabel
        anchors {
            top: parent.top
            left: parent.right
        }
        color: "#202020"
        smooth: true
        visible: number != ""
    }
}
