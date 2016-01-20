/***************************************************************************
 *   Copyright 2016 Michael Abrahams <miabraha@gmail.com>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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

Item {
    id: root;
    property variant predefinedTimers: plasmoid.configuration.predefinedTimers;

    property int seconds : restoreToSeconds(plasmoid.configuration.running, plasmoid.configuration.savedAt, plasmoid.configuration.seconds);

    // show title (can be customized in the settings dialog, default: disabled)
    property bool showTitle: plasmoid.configuration.showTitle;
    property string title: plasmoid.configuration.title;
    property bool running: (plasmoid.configuration.running > 0) ? true : false;
    property bool suspended: false;

    property string notificationText: plasmoid.configuration.notificationText;

    Plasmoid.toolTipMainText: {
        var timerName = "";
        if (showTitle && title != "") {
            timerName = title;
        } else {
            timerName = i18n("Timer");
        }

        var toolTipText = "";
        if (running) {
            toolTipText = i18n("%1 is running", timerName);
        } else {
            toolTipText = i18n("%1 not running", timerName);
        }
        return toolTipText;
    }
    Plasmoid.toolTipSubText:  i18np("Remaining time left: %1 second", "Remaining time left: %1 seconds", seconds);

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation
    Plasmoid.compactRepresentation: TimerView { }
    Plasmoid.fullRepresentation: TimerView { }

    PlasmaCore.DataSource {
        id: notificationSource
        engine: "notifications"
        connectedSources: "org.freedesktop.Notifications"
    }

    function createNotification() {
        var service = notificationSource.serviceForSource("notification");
        var operation = service.operationDescription("createNotification");

        operation.appName = root.title || i18n("Timer");
        operation["appIcon"] = "chronometer";
        operation.summary = notificationText || i18n("Timer finished")
        operation["body"] = "";
        operation["timeout"] = 2000;

        service.startOperationCall(operation);
    }

    Component.onCompleted: {
        plasmoid.setAction("timerStart", i18n("&Start"));
        plasmoid.setAction("timerStop", i18n("S&top"));
        plasmoid.setAction("timerReset", i18n("&Reset"));
        plasmoid.setActionSeparator("separator0");

        for (var predefinedTimer in plasmoid.configuration.predefinedTimers) {
            plasmoid.setAction("predefined_timer_" + plasmoid.configuration.predefinedTimers[predefinedTimer],
                               secondsToDisplayableString(plasmoid.configuration.predefinedTimers[predefinedTimer]));
        }
        plasmoid.setActionSeparator("separator1");
    }

    function secondsToDisplayableString(sec) {
        return ~~((sec / (60*60)) / 10) + "" +
            (~~(~~(sec / (60*60))) % 10) + ":" +
            ~~(~~((sec % (60*60)) / 60) / 10) + "" +
            ~~((sec % (60*60)) / 60) % 10 + ":" +
            ~~((sec % 60) / 10) + "" +
            (sec % 60) % 10;
    }
    function startTimer() {
        running = true;
        suspended = false;
        // timerDigits.opacity = 1.0;
        saveTimer();
    }

    function stopTimer() {
        running = false;
        suspended = true;
        saveTimer();
    }

    function resetTimer() {
        running = false;
        suspended = false;
        seconds = 0;
        saveTimer();
    }

    signal opacityNeedsReset()
    signal digitHasChanged()

    function saveTimer() {
        plasmoid.configuration.running = running ? seconds : 0;
        plasmoid.configuration.savedAt = new Date();
        plasmoid.configuration.seconds = seconds
    }

    function actionTriggered(actionName) {
        if (actionName.indexOf("predefined_timer_") == 0) {
            seconds = actionName.replace("predefined_timer_", "");
            startTimer();
        }
    }

    function restoreToSeconds(cRunning, cSavedAt, cSeconds) {
        if (cRunning > 0) {
            var elapsedSeconds = cRunning - ~~(~~(((new Date()).getTime() - cSavedAt.getTime()) / 1000));
            if (elapsedSeconds >= 0) {
                return elapsedSeconds;
            } else {
                return 0;
            }
        } else {
            return cSeconds;
        }
    }


    function action_timerStart() {
        startTimer();
    }

    function action_timerStop() {
        stopTimer();
    }

    function action_timerReset() {
        resetTimer();
    }

}

