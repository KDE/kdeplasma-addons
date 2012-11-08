/*****************************************************************************
 *   Copyright (C) 2012 by Davide Bettio <davide.bettio@kdemail.net>         *
 *   Copyright (C) 2012 by Luiz Romário Santana Rios <luizromario@gmail.com> *
 *   Copyright (C) 2007 by Henry Stanaland <stanaland@gmail.com>             *
 *   Copyright (C) 2008 by Laurent Montel  <montel@kde.org>                  *
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

import QtQuick 1.0;
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.locale 0.1 as Locale

Item {
    id: main;

    property int minimumWidth: 200;
    property int minimumHeight: 250;

    property int buttonHeight: (height - displayFrame.height - 6 * buttonsGrid.spacing) / 5;
    property int buttonWidth: (width / 4) - buttonsGrid.spacing;

    property real result: 0;
    property bool hasResult: false;
    property bool showingInput: true;
    property string operator: undefined;
    property real operand: 0;
    property bool commaPressed: false;
    property int decimals: 0;
    property int inputSize: 0;

    property int maxInputLength: 15; // More than that and the number notation turns scientific
                                     // (i.e.: 1.32324e+12)

    Keys.onDigit0Pressed: { digitClicked(0); }
    Keys.onDigit1Pressed: { digitClicked(1); }
    Keys.onDigit2Pressed: { digitClicked(2); }
    Keys.onDigit3Pressed: { digitClicked(3); }
    Keys.onDigit4Pressed: { digitClicked(4); }
    Keys.onDigit5Pressed: { digitClicked(5); }
    Keys.onDigit6Pressed: { digitClicked(6); }
    Keys.onDigit7Pressed: { digitClicked(7); }
    Keys.onDigit8Pressed: { digitClicked(8); }
    Keys.onDigit9Pressed: { digitClicked(9); }
    Keys.onEscapePressed: { allClearClicked(); }
    Keys.onDeletePressed: { clearClicked(); }
    Keys.onPressed: {
        switch (event.key) {
        case Qt.Key_Plus:
            setOperator("+");
            break;
        case Qt.Key_Minus:
            setOperator("-");
            break;
        case Qt.Key_Asterisk:
            setOperator("*");
            break;
        case Qt.Key_Slash:
            setOperator("/");
            break;
        case Qt.Key_Period:
            decimalClicked();
            break;
        case Qt.Key_Equal:
        case Qt.Key_Return:
        case Qt.Key_Enter:
            equalsClicked();
            break;
        default:
            break;
        }
    }

    function digitClicked(digit) {
        if (inputSize >= maxInputLength) {
            return;
        }

        if (!showingInput && hasResult) {
            allClearClicked();
        }

        if (commaPressed) {
            ++decimals;
            tenToTheDecimals = Math.pow(10, decimals);
            operand = (operand * tenToTheDecimals + digit) / tenToTheDecimals;
        } else {
            operand = operand * 10 + digit;
        }
        display.text = operand;
        showingInput = true;
        ++inputSize;
    }

    function decimalClicked() {
        commaPressed = true;
        showingInput = true;
    }

    function doOperation() {
        switch (operator) {
        case "+":
            result += operand;
            break;
        case "-":
            result -= operand;
            break;
        case "*":
            result *= operand;
            break;
        case "/":
            result /= operand;
            break;
        default:
            return;
        }

        display.text = algarismCount(result * Math.pow(10, decimals)) > maxInputLength?
            "E" : result;
        showingInput = false;
    }

    function clearOperand() {
        operand = 0;
        commaPressed = false;
        decimals = 0;
        inputSize = 0;
    }

    function setOperator(op) {
        if (!hasResult) {
            result = operand;
            hasResult = true;
        } else if (showingInput) {
            doOperation();
        }
        clearOperand();
        operator = op;
    }

    function equalsClicked() {
        doOperation();
    }

    function clearClicked() {
        clearOperand();
        operator = "";
        display.text = operand;
        showingInput = true;
    }

    function allClearClicked() {
        clearClicked();
        result = 0;
        hasResult = false;
        showingInput = true;
    }

    function algarismCount(number) {
        return number == 0? 1 :
                            Math.floor(Math.log(Math.abs(number))/Math.log(10)) + 1;
    }

    Locale.Locale {
        id: locale;
    }

    PlasmaCore.Theme {
        id: plasmaTheme;
    }

    Connections {
        target: plasmoid;
        onPopupEvent: {
            main.focus = true;
        }
    }

    Column {
        focus: true;
        spacing: 4;

        PlasmaCore.FrameSvgItem {
            id: displayFrame;
            width: buttonsGrid.width;
            height: 2 * display.font.pixelSize;
            imagePath: "widgets/frame";
            prefix: "sunken";

            TextEdit {
                id: display;
                anchors {
                    fill: parent;
                    margins: parent.margins.right;
                }
                text: "0";
                font.pointSize: 16;
                font.weight: Font.Bold;
                color: plasmaTheme.viewTextColor;
                horizontalAlignment: TextEdit.AlignRight;
                verticalAlignment: TextEdit.AlignVCenter;
                readOnly: true;
            }
        }

        Grid {
            id: buttonsGrid;
            columns: 4;
            rows: 5;
            spacing: 4;

            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: i18nc("Text of the clear button", "C");
                onClicked: clearClicked();
            }

            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: i18nc("Text of the division button", "÷");
                onClicked: setOperator("/");
            }

            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: i18nc("Text of the multiplication button", "×");
                onClicked: setOperator("*");
            }

            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: i18nc("Text of the all clear button", "AC");
                onClicked: allClearClicked();
            }


            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: "7";
                onClicked: digitClicked(7);
            }

            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: "8";
                onClicked: digitClicked(8);
            }

            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: "9";
                onClicked: digitClicked(9);
            }

            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: i18nc("Text of the minus button", "-");
                onClicked: setOperator("-");
            }


            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: "4";
                onClicked: digitClicked(4);
            }

            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: "5";
                onClicked: digitClicked(5);
            }

            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: "6";
                onClicked: digitClicked(6);
            }

            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: i18nc("Text of the plus button", "+");
                onClicked: setOperator("+");
            }


            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: "1";
                onClicked: digitClicked(1);
            }

            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: "2";
                onClicked: digitClicked(2);
            }

            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: "3";
                onClicked: digitClicked(3);
            }

            Item {
                id: ansPlaceHolder;
                width: buttonWidth;
                height: buttonHeight;
            }

            Item {
                id: zeroPlaceHolder;
                width: buttonWidth;
                height: buttonHeight;
            }

            Item {
                id: zeroPlaceHolder2;
                width: buttonWidth;
                height: buttonHeight;
            }

            PlasmaComponents.Button {
                width: buttonWidth;
                height: buttonHeight;
                text: locale.decimalSymbol;
                onClicked: decimalClicked();
            }

            Item {
                id: ansPlaceHolder2;
                width: buttonWidth;
                height: buttonHeight;
            }
        }
    }

    PlasmaComponents.Button {
        width: buttonWidth * 2 + buttonsGrid.spacing;
        height: buttonHeight;
        x: zeroPlaceHolder.x + buttonsGrid.x;
        y: zeroPlaceHolder.y + buttonsGrid.y;
        text: "0";
        onClicked: digitClicked(0);
    }

    PlasmaComponents.Button {
        id: ansButton;
        width: buttonWidth;
        height: buttonHeight * 2 + buttonsGrid.spacing;
        x: ansPlaceHolder.x + buttonsGrid.x;
        y: ansPlaceHolder.y + buttonsGrid.y;
        text: i18nc("Text of the equals button", "=");
        onClicked: equalsClicked();
    }
}

