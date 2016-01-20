/***************************************************************************
 *   Copyright 2008,2014 by Davide Bettio <davide.bettio@kdemail.net>      *
 *   Copyright 2015 by Bernhard Friedreich <friesoft@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.2
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as QtExtra
import org.kde.plasma.private.timer 0.1 as TimerPlasmoid

Item {
    id: main

    property date savedAt: plasmoid.configuration.savedAt;

    // display seconds in addition to hours and minutes (default: enabled)
    property bool showSeconds: plasmoid.configuration.showSeconds;

    // show notification on timer completion (default: enabled)
    property bool showNotification: plasmoid.configuration.showNotification;

    // run custom command on timer completion (default: disabled)
    property bool runCommand: plasmoid.configuration.runCommand;
    property string command: plasmoid.configuration.command;

    property real digits: (showSeconds) ? 7 : 4.5;
    property int digitH: ((height / 2) * digits < width ? height : ((width - (digits - 1)) / digits) * 2);
    property int digitW: digitH / 2;

    PlasmaCore.Svg {
        id: timerSvg
        imagePath: "widgets/timer"
    }

    Timer {
        id: t;
        interval: 1000;
        onTriggered: {
            if (root.seconds != 0) {
                root.seconds--;
            }
            if (root.seconds == 0) {
                root.running = false;

                if (showNotification) {
                    root.createNotification();
                }
                if (runCommand) {
                    TimerPlasmoid.Timer.runCommand(command);
                }
                saveTimer();
            }
        }
        repeat: true;
        running: root.running;
    }

    Timer {
        id: delayedSaveTimer;
        interval: 3000;
        onTriggered: saveTimer();
    }

    Column {
        anchors.centerIn: parent;
        Text {
            id: titleLabel;
            text: root.title;
            visible: root.showTitle;
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: parent.parent.height - digitH;
        }
        Row {
            id: timerDigits;
            SequentialAnimation on opacity {
                running: root.suspended;
                loops: Animation.Infinite;
                NumberAnimation {
                    duration: 800;
                    from: 1.0;
                    to: 0.2;
                    easing.type: Easing.InOutQuad;
                }
                PauseAnimation {
                    duration: 400;
                }
                NumberAnimation {
                    duration: 800;
                    from: 0.2;
                    to: 1.0;
                    easing.type: Easing.InOutQuad;
                }
                PauseAnimation {
                    duration: 400;
                }
            }

            TimerDigit {
                meaning: 60*60*10;
                num: ~~((root.seconds / (60*60)) / 10);
                suffix: (root.running && root.seconds < 60) ? "_1" : "";
            }
            TimerDigit {
                meaning: 60*60;
                num: ~~(~~(root.seconds / (60*60))) % 10;
                suffix: (root.running && root.seconds < 60) ? "_1" : "";
            }

            PlasmaCore.SvgItem {
                svg: timerSvg;
                width: digitW / 2;
                height: digitH;
                elementId: "separator" + ((root.running && root.seconds < 60) ? "_1" : "");
            }
            TimerDigit {
                meaning: 600;
                num: ~~(~~((root.seconds % (60*60)) / 60) / 10);
                suffix: (root.running && root.seconds < 60) ? "_1" : "";
            }
            TimerDigit {
                meaning: 60;
                num: ~~((root.seconds % (60*60)) / 60) % 10;
                suffix: (root.running && root.seconds < 60) ? "_1" : "";
            }

            PlasmaCore.SvgItem {
                svg: timerSvg;
                width: digitW / 2;
                height: digitH;
                elementId: "separator" + ((root.running && root.seconds < 60) ? "_1" : "");
                visible: showSeconds;
            }
            TimerDigit {
                meaning: 10;
                num: ~~((root.seconds % 60) / 10);
                suffix: (root.running && root.seconds < 60) ? "_1" : "";
                visible: showSeconds;
            }
            TimerDigit {
                meaning: 1;
                num: (root.seconds % 60) % 10;
                suffix: (root.running && root.seconds < 60) ? "_1" : "";
                visible: showSeconds;
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

    function digitChanged() {
        delayedSaveTimer.stop();
        delayedSaveTimer.start();
    }

    function resetOpacity() {
        timerDigits.opacity = 1.0;
    }

    Component.onCompleted: {
        root.opacityNeedsReset.connect(resetOpacity);
        root.digitHasChanged.connect(digitChanged);
    }

}
