/***************************************************************************
 *   Copyright 2008,2014 by Davide Bettio <davide.bettio@kdemail.net>      *
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
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as QtExtra

Item
{
    id: root;

    property int seconds : restoreToSeconds(plasmoid.configuration.running, plasmoid.configuration.savedAt, plasmoid.configuration.seconds);
    property bool running: (plasmoid.configuration.running > 0) ? true : false;
    property variant predefinedTimers: plasmoid.configuration.predefinedTimers;
    property date savedAt: plasmoid.configuration.savedAt;
    property bool showTitle: plasmoid.configuration.showTitle;
    property string title: plasmoid.configuration.title;
    property bool hideSeconds: plasmoid.configuration.hideSeconds;
    property bool showMessage: plasmoid.configuration.showMessage;
    property string message: plasmoid.configuration.message;
    property bool runCommand: plasmoid.configuration.runCommand;
    property string command: plasmoid.configuration.command;
    property real digits: (hideSeconds) ? 4.5 : 7;
    property int digitH: ((height / 2) * digits < width ? height : ((width - (digits - 1)) / digits) * 2);
    property int digitW: digitH / 2;
    property bool suspended: false;

    PlasmaCore.Svg {
        id: timerSvg
        imagePath: "widgets/timer"
    }

    Timer {
        id: t;
        interval: 1000;
        onTriggered:{
            if (seconds != 0){
                seconds--;
            }
            if (seconds == 0){
                parent.running = false;
                saveTimer();
            }
        }
        repeat: true;
        running: parent.running;
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
            text: title;
            visible: showTitle;
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
                    easing: Easing.InOutQuad;
                }
                PauseAnimation {
                    duration: 400;
                }
                NumberAnimation {
                    duration: 800;
                    from: 0.2;
                    to: 1.0;
                    easing: Easing.InOutQuad;
                }
                PauseAnimation {
                    duration: 400;
                }
            }

            TimerDigit {
                meaning: 60*60*10;
                num: ~~((seconds / (60*60)) / 10);
                suffix: (running && seconds < 60) ? "_1" : "";
            }
            TimerDigit {
                meaning: 60*60;
                num: ~~(~~(seconds / (60*60))) % 10;
                suffix: (running && seconds < 60) ? "_1" : "";
            }

            PlasmaCore.SvgItem {
                svg: timerSvg;
                width: digitW / 2;
                height: digitH;
                elementId: "separator" + ((running && seconds < 60) ? "_1" : "");
            }
            TimerDigit {
                meaning: 600;
                num: ~~(~~((seconds % (60*60)) / 60) / 10);
                suffix: (running && seconds < 60) ? "_1" : "";
            }
            TimerDigit {
                meaning: 60;
                num: ~~((seconds % (60*60)) / 60) % 10;
                suffix: (running && seconds < 60) ? "_1" : "";
            }

            PlasmaCore.SvgItem {
                svg: timerSvg;
                width: digitW / 2;
                height: digitH;
                elementId: "separator" + ((running && seconds < 60) ? "_1" : "");
                visible: !hideSeconds;
            }
            TimerDigit {
                meaning: 10;
                num: ~~((seconds % 60) / 10);
                suffix: (running && seconds < 60) ? "_1" : "";
                visible: !hideSeconds;
            }
            TimerDigit {
                meaning: 1;
                num: (seconds % 60) % 10;
                suffix: (running && seconds < 60) ? "_1" : "";
                visible: !hideSeconds;
            }
        }
    }

    MouseArea {
        anchors.fill: parent;
        onClicked:{
            if (parent.running){
                 stopTimer();
            }else{
                 startTimer();
            }
        }
    }

    Component.onCompleted: {
        plasmoid.setAction("timerStart", i18n("&Start"));
        plasmoid.setAction("timerStop", i18n("S&top"));
        plasmoid.setAction("timerReset", i18n("&Reset"));
        plasmoid.setActionSeparator("separator0");

        for (var predefinedTimer in plasmoid.configuration.predefinedTimers){
            plasmoid.setAction("predefined_timer_" + plasmoid.configuration.predefinedTimers[predefinedTimer],
                               secondsToDisplayableString(plasmoid.configuration.predefinedTimers[predefinedTimer]));
        }
        plasmoid.setActionSeparator("separator1");
    }

    function secondsToDisplayableString(sec)
    {

                return ~~((sec / (60*60)) / 10) + "" +
                (~~(~~(sec / (60*60))) % 10) + ":" +
                ~~(~~((sec % (60*60)) / 60) / 10) + "" +
                ~~((sec % (60*60)) / 60) % 10 + ":" +
                ~~((sec % 60) / 10) + "" +
                (sec % 60) % 10;
    }

    function startTimer()
    {
        running = true;
        suspended = false;
        timerDigits.opacity = 1.0;
        saveTimer();
    }

    function stopTimer()
    {
        running = false;
        suspended = true;
        saveTimer();
    }

    function resetTimer()
    {
        running = false;
        suspended = false;
        seconds = 0;
        timerDigits.opacity = 1.0;
        saveTimer();
    }

    function saveTimer()
    {
        plasmoid.configuration.running = running ? seconds : 0;
        plasmoid.configuration.savedAt = new Date();
        plasmoid.configuration.seconds = seconds
    }

    function restoreToSeconds(cRunning, cSavedAt, cSeconds)
    {
        if (cRunning > 0){
            var elapsedSeconds = cRunning - ~~(~~(((new Date()).getTime() - cSavedAt.getTime()) / 1000));
            if (elapsedSeconds >= 0){
                return elapsedSeconds;
            }else{
                return 0;
            }
        }else{
            return cSeconds;
        }
    }

    function digitChanged()
    {
        delayedSaveTimer.stop();
        delayedSaveTimer.start();
    }

    function actionTriggered(actionName)
    {
        if (actionName.indexOf("predefined_timer_") == 0){
            seconds = actionName.replace("predefined_timer_", "");
            startTimer();
        }
    }

    function action_timerStart()
    {
        startTimer();
    }

    function action_timerStop()
    {
        stopTimer();
    }

    function action_timerReset()
    {
        resetTimer();
    }
}

