/*
 *  SPDX-FileCopyrightText: 2020 Łukasz Korbel <corebell.it@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
pragma ComponentBehavior: Bound

import QtQuick
import org.kde.ksvg as KSvg
import plasma.applet.org.kde.plasma.timer as TimerPlasmoid

Row {
    id: timerEdit

    required property real maximumHeight
    required property real maximumWidth

    property int value // time in seconds
    property bool showSeconds: true
    property bool editable: true
    // in alert mode 2nd set of digits from svg file will be drawn
    property bool alertMode: false

    signal digitModified(int valueDelta)
    signal toggleTimerRequested()

    height: internal.digitH
    width: internal.digitW * internal.digits

    QtObject {
        id: internal
        readonly property string digitSuffix: timerEdit.alertMode ? "_1" : ""
        // digits count include separators with 50% of digit width
        readonly property real digits: timerEdit.showSeconds ? 7 : 4.5;
        readonly property int digitH: calculateDigitHeight(timerEdit.maximumHeight, timerEdit.maximumWidth)
        readonly property int digitW: digitH / 2;
        readonly property string valueString: TimerPlasmoid.Timer.secondsToString(timerEdit.value, "hhmmss") // "000000"

        function calculateDigitHeight(maxHeight: int, maxWidth: int) : int {
            return (maxHeight / 2) * digits < maxWidth ? maxHeight : maxWidth / digits * 2
        }
    }

    component Digit: KSvg.SvgItem {
        id: digitSvg

        required property int meaning
        required property int num
        readonly property int mouseWheelAngleThreshold: 5

        width: internal.digitW
        height: internal.digitH

        activeFocusOnTab: true
        imagePath: "widgets/timer"
        elementId: num + internal.digitSuffix

        Keys.onPressed: event => {
            switch (event.key) {
            case Qt.Key_Return:
            case Qt.Key_Enter:
            case Qt.Key_Space:
            case Qt.Key_Select:
                timerEdit.toggleTimerRequested();
                break;
            case Qt.Key_Up:
                if (timerEdit.value + meaning < 24*60*60) {
                    timerEdit.digitModified(meaning)
                }
                break;
            case Qt.Key_Down:
                if (timerEdit.value - meaning >= 0) {
                    timerEdit.digitModified(-meaning)
                }
                break;
            case Qt.Key_Left:
                digitSvg.nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason);
                break;
            case Qt.Key_Right:
                digitSvg.nextItemInFocusChain(true).forceActiveFocus(Qt.TabFocusReason);
                break;
            default:
                return;
            }
            event.accepted = true;
        }

        MouseArea {
            anchors.fill: parent
            enabled: timerEdit.editable
            propagateComposedEvents: true

            onClicked: timerEdit.toggleTimerRequested()
            onWheel: wheel => {
                wheel.accepted = true
                if (wheel.angleDelta.y > digitSvg.mouseWheelAngleThreshold) {
                    if (timerEdit.value + digitSvg.meaning < 24*60*60) {
                        timerEdit.digitModified(digitSvg.meaning)
                    }
                } else if (wheel.angleDelta.y < -digitSvg.mouseWheelAngleThreshold) {
                    if (timerEdit.value - digitSvg.meaning >= 0) {
                        timerEdit.digitModified(-digitSvg.meaning)
                    }
                }
            }
        }
    }

    component DigitSeparator: KSvg.SvgItem {
        width: internal.digitW / 2;
        height: internal.digitH;
        imagePath: "widgets/timer"
        elementId: "separator" + internal.digitSuffix
    }

    // Following 8 Digit(Separators) will prepare all elements
    // to display time in format "hh:mm:ss"

    Digit {
        id: hour1
        meaning: 60*60*10 // 10h
        num: internal.valueString[0]
    }

    Digit {
        id: hour2
        meaning: 60*60 // 1h
        num: internal.valueString[1]
    }

    DigitSeparator { } // :

    Digit {
        id: minute1
        meaning: 600 // 10min
        num: internal.valueString[2]
    }
    Digit {
        id: minute2
        meaning: 60 // 1min
        num: internal.valueString[3]
    }

    DigitSeparator { visible: timerEdit.showSeconds } // ":"

    Digit {
        id: second1
        visible: timerEdit.showSeconds
        meaning: 10 // 10s
        num: internal.valueString[4]
    }
    Digit {
        id: second2
        visible: timerEdit.showSeconds
        meaning: 1 // 1s
        num: internal.valueString[5]
    }
}

