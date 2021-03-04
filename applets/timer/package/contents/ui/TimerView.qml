/*
 *   SPDX-FileCopyrightText: 2008, 2014 Davide Bettio <davide.bettio@kdemail.net>
 *   SPDX-FileCopyrightText: 2015 Bernhard Friedreich <friesoft@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.2
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrolsaddons 2.0 as QtExtra

Item {
    id: main
    readonly property int secondsForAlert: 60

    Column {

        Text {
            id: titleLabel
            text: root.title
            visible: root.showTitle;
            horizontalAlignment: Text.AlignHCenter
            height: 0.25 * main.height
            font.pixelSize: 0.5 * height
        }
        
        TimerEdit {
            id: timerDigits
            value: root.seconds
            editable: !root.running
            alertMode: root.running && (root.seconds < main.secondsForAlert)
            width: main.width
            height: main.height - titleLabel.height
            onDigitModified: root.seconds += valueDelta
            SequentialAnimation on opacity {
                running: root.suspended;
                loops: Animation.Infinite;
                NumberAnimation {
                    duration: PlasmaCore.Units.veryLongDuration * 2;
                    from: 1.0;
                    to: 0.2;
                    easing.type: Easing.InOutQuad;
                }
                PauseAnimation {
                    duration: PlasmaCore.Units.veryLongDuration;
                }
                NumberAnimation {
                    duration: PlasmaCore.Units.veryLongDuration * 2;
                    from: 0.2;
                    to: 1.0;
                    easing.type: Easing.InOutQuad;
                }
                PauseAnimation {
                    duration: PlasmaCore.Units.veryLongDuration;
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent;
        onClicked: {
            if (root.running) {
                 root.stopTimer();
            } else {
                 root.startTimer();
            }
        }
        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            subText: root.running ? i18n("Timer is running") : i18n("Use mouse wheel to change digits or choose from predefined timers in the context menu");
        }
    }

    function resetOpacity() {
        timerDigits.opacity = 1.0;
    }

    Component.onCompleted: {
        root.opacityNeedsReset.connect(resetOpacity);
    }

}
