/*****************************************************************************
 *   Copyright (C) 2012, 2014 by Davide Bettio <davide.bettio@kdemail.net>   *
 *   Copyright (C) 2012, 2014 by David Edmundson <davidedmundson@kde.org >   *
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

import QtQuick 2.2
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as QtExtra
import org.kde.plasma.plasmoid 2.0

Item {
    id: main;

    Plasmoid.switchWidth: Layout.minimumWidth
    Plasmoid.switchHeight: Layout.minimumHeight
    Layout.minimumWidth: mainLayout.Layout.minimumWidth + 8
    Layout.minimumHeight: mainLayout.Layout.minimumHeight + 8

    width: units.gridSize * 3
    height: units.gridSize * 4

    property real result: 0;
    property bool hasResult: false;
    property bool showingInput: true;
    property bool showingResult: false;
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

        if (showingResult) {
            allClearClicked();
        }

        if (commaPressed) {
            ++decimals;
            var tenToTheDecimals = Math.pow(10, decimals);
            operand = (operand * tenToTheDecimals + digit) / tenToTheDecimals;
        } else {
            operand = operand * 10 + digit;
        }
        displayNumber(operand);
        showingInput = true;
        ++inputSize;
    }

    function decimalClicked() {
        if (showingResult) {
            allClearClicked();
        }

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

        displayNumber(algarismCount(result * Math.pow(10, decimals)) > maxInputLength?
            "E" : result);
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
        showingResult = false;
    }

    function equalsClicked() {
        showingResult = true;
        doOperation();
    }

    function clearClicked() {
        clearOperand();
        operator = "";
        displayNumber(operand);
        showingInput = true;
        showingResult = false;
    }

    function allClearClicked() {
        clearClicked();
        result = 0;
        hasResult = false;
    }

    function algarismCount(number) {
        return number == 0? 1 :
                            Math.floor(Math.log(Math.abs(number))/Math.log(10)) + 1;
    }

    function displayNumber(number){
        display.text = number.toString().replace(".", Qt.locale().decimalPoint);
    }

    Connections {
        target: plasmoid;
        onPopupEvent: {
            main.focus = true;
        }
    }

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 4

        focus: true;
        spacing: 4;

        PlasmaCore.FrameSvgItem {
            id: displayFrame;
            Layout.fillWidth: true
            Layout.minimumHeight: 2 * display.font.pixelSize;
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
                color: theme.viewTextColor;
                horizontalAlignment: TextEdit.AlignRight;
                verticalAlignment: TextEdit.AlignVCenter;
                readOnly: true;
            }
        }

        GridLayout {
            id: buttonsGrid;
            columns: 4;
            rows: 5;
            columnSpacing: 4
            rowSpacing: 4

            Layout.fillWidth: true
            Layout.fillHeight: true

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: i18nc("Text of the clear button", "C");
                onClicked: clearClicked();
            }

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: i18nc("Text of the division button", "÷");
                onClicked: setOperator("/");
            }

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: i18nc("Text of the multiplication button", "×");
                onClicked: setOperator("*");
            }

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: i18nc("Text of the all clear button", "AC");
                onClicked: allClearClicked();
            }


            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: "7";
                onClicked: digitClicked(7);
            }

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: "8";
                onClicked: digitClicked(8);
            }

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: "9";
                onClicked: digitClicked(9);
            }

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: i18nc("Text of the minus button", "-");
                onClicked: setOperator("-");
            }


            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: "4";
                onClicked: digitClicked(4);
            }

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: "5";
                onClicked: digitClicked(5);
            }

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: "6";
                onClicked: digitClicked(6);
            }

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: i18nc("Text of the plus button", "+");
                onClicked: setOperator("+");
            }


            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: "1";
                onClicked: digitClicked(1);
            }

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: "2";
                onClicked: digitClicked(2);
            }

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: "3";
                onClicked: digitClicked(3);
            }

            PlasmaComponents.Button {
                id: ansButton;
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                Layout.rowSpan: 2
                text: i18nc("Text of the equals button", "=");
                onClicked: equalsClicked();
            }

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                Layout.columnSpan: 2
                text: "0";
                onClicked: digitClicked(0);
            }

            PlasmaComponents.Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: minimumWidth
                Layout.minimumHeight: minimumHeight
                text: Qt.locale().decimalPoint;
                onClicked: decimalClicked();
            }
        }
    }
}

