/*
 *   SPDX-FileCopyrightText: 2016 Michael Abrahams <miabraha@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.2
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrolsaddons 2.0 as QtExtra
import org.kde.plasma.private.timer 0.1 as TimerPlasmoid

Item {
    id: root;
    readonly property variant predefinedTimers: plasmoid.configuration.predefinedTimers;

    Plasmoid.backgroundHints: PlasmaCore.Types.ShadowBackground | PlasmaCore.Types.ConfigurableBackground

    property int seconds : restoreToSeconds(plasmoid.configuration.running, plasmoid.configuration.savedAt, plasmoid.configuration.seconds);

    // show notification on timer completion (default: enabled)
    property bool showNotification: plasmoid.configuration.showNotification;
    // run custom command on timer completion (default: disabled)
    property bool runCommand: plasmoid.configuration.runCommand;
    property string command: plasmoid.configuration.command;

    // show title (can be customized in the settings dialog, default: disabled)
    readonly property bool showTitle: plasmoid.configuration.showTitle;
    readonly property string title: plasmoid.configuration.title;
    property bool running: (plasmoid.configuration.running > 0) ? true : false;
    property bool suspended: false;

    readonly property string notificationText: plasmoid.configuration.notificationText;

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

    function onDigitHasChanged() {
        delayedSaveTimer.stop();
        delayedSaveTimer.start();
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

    Component.onCompleted: rebuildMenu()

    Connections {
        target: plasmoid.configuration
        onPredefinedTimersChanged: rebuildMenu()
    }

    function rebuildMenu() {
        plasmoid.clearActions();
        plasmoid.setAction("timerStart", i18nc("@action", "&Start"));
        plasmoid.setAction("timerStop", i18nc("@action", "S&top"));
        plasmoid.setAction("timerReset", i18nc("@action", "&Reset"));
        plasmoid.setActionSeparator("separator0");

        for (var predefinedTimer of plasmoid.configuration.predefinedTimers) {
            plasmoid.setAction("predefined_timer_" + predefinedTimer,
                               TimerPlasmoid.Timer.secondsToString(predefinedTimer, "hh:mm:ss"));
        }
        plasmoid.setActionSeparator("separator1");
    }

    function startTimer() {
        running = true;
        suspended = false;
        opacityNeedsReset();
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
        opacityNeedsReset();
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
        if (actionName.indexOf("predefined_timer_") === 0) {
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

