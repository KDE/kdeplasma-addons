/*
 *   SPDX-FileCopyrightText: 2016 Michael Abrahams <miabraha@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-3.0-or-later
 */
pragma ComponentBehavior: Bound

import QtQuick
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import plasma.applet.org.kde.plasma.timer as TimerPlasmoid
import org.kde.notification

PlasmoidItem {
    id: root;

    width: Math.round(switchWidth * 1.5)
    height: Math.round(switchHeight * 1.5)

    switchWidth: Kirigami.Units.gridUnit * 10
    switchHeight: Kirigami.Units.gridUnit * 4

    readonly property bool inPanel: [PlasmaCore.Types.TopEdge, PlasmaCore.Types.RightEdge, PlasmaCore.Types.BottomEdge, PlasmaCore.Types.LeftEdge]
        .includes(Plasmoid.location)
    readonly property bool isVertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical

    Plasmoid.backgroundHints: PlasmaCore.Types.ShadowBackground | PlasmaCore.Types.ConfigurableBackground

    property int seconds : restoreToSeconds(Plasmoid.configuration.running, Plasmoid.configuration.savedAt, Plasmoid.configuration.seconds, Plasmoid.configuration.suspendedAtSecond);

    readonly property bool alertMode: root.running && root.seconds < 60
    property bool running: Plasmoid.configuration.running > 0
    property bool suspended: Plasmoid.configuration.suspendedAtSecond != -1;

    toolTipMainText: {
        const timerName = (Plasmoid.configuration.showTitle && Plasmoid.configuration.title) || Plasmoid.title;

        if (running) {
            return i18nc("@info:tooltip for widget, %1 is timer name", "%1 is running", timerName);
        } else {
            return i18nc("@info:tooltip for widget, %1 is timer name", "%1 not running", timerName);
        }
    }
    toolTipSubText: running
        ? i18ncp("@info:tooltip for widget, subtitle, %1 is remaining time in seconds for running timer", "Remaining time left: %1 second", "Remaining time left: %1 seconds", seconds)
        : i18nc("@info:tooltip for widget, subtitle, timer not running", "Use mouse wheel to change digits or choose from predefined timers in the context menu");

    compactRepresentation: CompactRepresentation {
        inPanel: root.inPanel
        isVertical: root.isVertical
        running: root.running
        alertMode: root.alertMode
        seconds: root.seconds
        onToggleExpandedRequested: root.expanded = !root.expanded
        onToggleTimerRequested: root.toggleTimer()
        onAdjustTimerRequested: delta => root.adjustTimer(delta)
    }
    fullRepresentation: TimerView {
        inPanel: root.inPanel
        isVertical: root.isVertical
        suspended: root.suspended
        running: root.running
        alertMode: root.alertMode
        seconds: root.seconds
        onToggleTimerRequested: root.toggleTimer()
        onAdjustTimerRequested: delta => root.adjustTimer(delta)
    }

    function toggleTimer() {
        if (root.running) {
            root.stopTimer();
        } else {
            root.startTimer();
        }
    }

    Notification {
        id: timerNotification
        componentName: "plasma_applet_timer"
        eventId: "timerFinished"
        title: Plasmoid.configuration.title || i18nc("@title:window notification title with no custom timer name set", "Timer")
        text: Plasmoid.configuration.notificationText || i18nc("@info:progress notification body when no custom message set", "Timer finished")
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

                if (Plasmoid.configuration.showNotification) {
                    timerNotification.sendEvent();
                }
                if (Plasmoid.configuration.runCommand) {
                    TimerPlasmoid.Timer.runCommand(Plasmoid.configuration.command);
                }
                root.saveTimer();
            }
        }
        repeat: true;
        running: root.running;
    }

    Timer {
        id: delayedSaveTimer;
        interval: 3000;
        onTriggered: root.saveTimer();
    }

    function onDigitHasChanged() {
        delayedSaveTimer.stop();
        delayedSaveTimer.start();
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            id: startAction
            text: root.seconds > 0 && !root.running
                ? i18nc("@action %1 is the duration that will be started, e.g. 00:05:00", "&Start (%1)", TimerPlasmoid.Timer.secondsToString(root.seconds, "hh:mm:ss"))
                : i18nc("@action", "&Start")
            enabled: !root.running && (root.seconds !== 0 || Plasmoid.configuration.seconds !== 0)
            onTriggered: root.startTimer()
        },
        PlasmaCore.Action {
            id: stopAction
            text: i18nc("@action", "S&top")
            enabled: root.running
            onTriggered: root.stopTimer()
        },
        PlasmaCore.Action {
            id: resetAction
            text: i18nc("@action", "&Reset")
            enabled: root.seconds !== 0 && Plasmoid.configuration.seconds !== 0
            onTriggered: root.resetTimer()
        },
        PlasmaCore.Action {
            id: separator1
            isSeparator: true
        },
        PlasmaCore.Action {
            id: separator2
            isSeparator: true
        }
    ]

    Instantiator {
        model: Plasmoid.configuration.predefinedTimers
        delegate: PlasmaCore.Action {
            required property int modelData
            text: TimerPlasmoid.Timer.secondsToString(modelData, "hh:mm:ss")
            onTriggered: {
                Plasmoid.configuration.seconds = modelData
                root.seconds = modelData;
                root.suspended = false
                root.startTimer();
            }
        }
        onObjectAdded: (index, object) => {
            Plasmoid.contextualActions.splice(Plasmoid.contextualActions.indexOf(separator2), 0, object)
        }
        onObjectRemoved: (index, object) => {
            Plasmoid.contextualActions.splice(Plasmoid.contextualActions.indexOf(object), 1)
        }
    }

    function startTimer() {
        running = true;
        // unset suspended in saveTimer - we don't want to set a new seconds value if restarting from suspend
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
        Plasmoid.configuration.seconds = 0;
        seconds = 0;
        opacityNeedsReset();
        saveTimer();
    }

    function adjustTimer(delta: int) {
        Plasmoid.configuration.seconds = Math.max(0, Math.min(Plasmoid.configuration.seconds + delta, 24*60*60 - 1))
        root.seconds = Math.max(Math.max(0, Math.min(root.seconds + delta, 24*60*60 - 1)));
    }

    signal opacityNeedsReset()
    signal digitHasChanged()

    function saveTimer() {
        Plasmoid.configuration.running = running ? seconds : 0;
        Plasmoid.configuration.savedAt = new Date();
        if (suspended && !running) {
            Plasmoid.configuration.suspendedAtSecond = seconds;
        } else {
            if (!suspended) {
                Plasmoid.configuration.seconds = seconds;
            }
            Plasmoid.configuration.suspendedAtSecond = -1;
            suspended = false;
        }
    }

    function restoreToSeconds(cRunning, cSavedAt, cSeconds, cSuspendedAtSecond) {
        if (cSuspendedAtSecond > -1 && cSuspendedAtSecond <= cSeconds) {
            return cSuspendedAtSecond;
        }
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
}

