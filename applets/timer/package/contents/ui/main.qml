/*
 *   SPDX-FileCopyrightText: 2016 Michael Abrahams <miabraha@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.15
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrolsaddons 2.0 as QtExtra
import org.kde.plasma.private.timer 0.1 as TimerPlasmoid

Item {
    id: root;

    Plasmoid.switchWidth: PlasmaCore.Units.gridUnit * 8
    Plasmoid.switchHeight: PlasmaCore.Units.gridUnit * 4

    readonly property bool inPanel: [PlasmaCore.Types.TopEdge, PlasmaCore.Types.RightEdge, PlasmaCore.Types.BottomEdge, PlasmaCore.Types.LeftEdge]
        .includes(Plasmoid.location)
    readonly property bool isVertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical

    readonly property variant predefinedTimers: plasmoid.configuration.predefinedTimers;

    Plasmoid.backgroundHints: PlasmaCore.Types.ShadowBackground | PlasmaCore.Types.ConfigurableBackground

    // Display remaining time (hours and minutes) (default: enabled)
    readonly property bool showRemainingTime: Plasmoid.configuration.showRemainingTime

    // Display seconds in addition to hours and minutes (default: enabled)
    readonly property bool showSeconds: Plasmoid.configuration.showSeconds
    property int seconds : restoreToSeconds(plasmoid.configuration.running, plasmoid.configuration.savedAt, plasmoid.configuration.seconds);

    // Display timer toggle control (default: enabled)
    readonly property bool showTimerToggle: Plasmoid.configuration.showTimerToggle

    // Display progress bar (default: disabled)
    readonly property bool showProgressBar: Plasmoid.configuration.showProgressBar

    // show notification on timer completion (default: enabled)
    property bool showNotification: plasmoid.configuration.showNotification;
    // run custom command on timer completion (default: disabled)
    property bool runCommand: plasmoid.configuration.runCommand;
    property string command: plasmoid.configuration.command;

    // show title (can be customized in the settings dialog, default: disabled)
    readonly property bool showTitle: plasmoid.configuration.showTitle;
    readonly property string title: plasmoid.configuration.title;
    readonly property bool alertMode: root.running && root.seconds < 60
    property bool running: (plasmoid.configuration.running > 0) ? true : false;
    property bool suspended: false;

    readonly property string notificationText: plasmoid.configuration.notificationText;

    Plasmoid.toolTipMainText: {
        var timerName = "";
        if (showTitle && title != "") {
            timerName = title;
        } else {
            timerName = Plasmoid.title;
        }

        var toolTipText = "";
        if (running) {
            toolTipText = i18n("%1 is running", timerName);
        } else {
            toolTipText = i18n("%1 not running", timerName);
        }
        return toolTipText;
    }
    Plasmoid.toolTipSubText: running ? i18np("Remaining time left: %1 second", "Remaining time left: %1 seconds", seconds) : i18n("Use mouse wheel to change digits or choose from predefined timers in the context menu");

    Plasmoid.compactRepresentation: CompactRepresentation { }
    Plasmoid.fullRepresentation: TimerView { }

    function toggleTimer() {
        if (root.running) {
            root.stopTimer();
        } else {
            root.startTimer();
        }
    }

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

