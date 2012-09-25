/*****************************************************************************
 *   Copyright (C) 2007 by Henry Stanaland <stanaland@gmail.com>             *
 *   Copyright (C) 2008 by Laurent Montel  <montel@kde.org>                  *
 *   Copyright (C) 2012 by Luiz Romário Santana Rios <luizromario@gmail.com> *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .          *
 *****************************************************************************/

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    property int minimumWidth: 200
    property int minimumHeight: 200
    property double result: 0
    property double current: 0
    property int digits: 0

    PlasmaComponents.TextArea {
        id: resultArea
        readOnly: true
        horizontalAlignment: TextEdit.AlignRight
        verticalAlignment: TextEdit.AlignVCenter
        height: 40

        font {
            bold: true
            pointSize: 16
        }

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 2
        }

        text: "0"
    }

    Grid {
        id: grid

        columns: 4
        spacing: 2

        anchors {
            top: resultArea.bottom
            bottom: button0.bottom
            left: parent.left
            right: parent.right
            margins: 2
        }

        Repeater {
            model: ["C", "÷", "×", "AC",
                    "7", "8", "9", "-",
                    "4", "5", "6", "+",
                    "1", "2", "3", "=",
                    "0", "."]

            PlasmaComponents.Button {
                id: button
                text: modelData
                width: (parent.width / parent.columns) - parent.spacing

                onClicked: {
                    switch (index) {
                    case 0:
                        current = 0;
                        digits = 0;
                        break;
                    case 1:
                        break;
                    case 2:
                        break;
                    case 3:
                        current = 0;
                        result = 0;
                        digits = 0;
                        break;
                    case 4:
                    case 5:
                    case 6:
                        if (digits < 15) {
                            current *= 10;
                            current += index + 3;
                            ++digits;
                        }
                        break;
                    case 7:
                        break;
                    case 8:
                    case 9:
                    case 10:
                        if (digits < 15) {
                            current *= 10;
                            current += index - 4;
                            ++digits;
                        }
                        break;
                    case 11:
                        break;
                    case 12:
                    case 13:
                    case 14:
                        if (digits < 15) {
                            current *= 10;
                            current += index - 11;
                            ++digits;
                        }
                        break;
                    case 15:
                        break;
                    case 16:
                        if (digits > 0 && digits < 15) {
                            current *= 10;
                            ++digits;
                        }
                        break;
                    case 17:
                        break;
                    default:
                        break;
                    }

                    resultArea.text = current;
                }
            }
        }
    }
}
