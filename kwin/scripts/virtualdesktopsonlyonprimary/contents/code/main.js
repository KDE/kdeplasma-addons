/*
    SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

// This script sets all windows on non-primary screens to "on all desktops",
// effectively making it so only the primary screen has virtual desktops.

/**
 * Prints a message to the console.
 * 
 * Other print methods are not showing up, so we are using console.info.
 * 
 * We are including a prefix to make it easier to find the messages.
 * 
 * Monitor output with:
 * $ journalctl -b -f | grep -i "virtualdesktopsonlyonprimary:"
 * 
 * @param {string} message The message to print.
 * @returns {void}
 */
function print(message) {
    console.info("virtualdesktopsonlyonprimary: " + message);
}

let primaryScreen = workspace.screenOrder[0];

/** 
 * Track which windows we're monitoring, so we don't connect multiple times.
 */
const windowMap = new Map();

function processWindow(window) {
    if (!window.normalWindow) return;

    // The virtual desktops this window is on. If it's on all desktops, the list is empty.
    let desktops = window.desktops;

    let onCurrentDesktop = desktops.includes(workspace.currentDesktop);
    let windowIsRelevant = onCurrentDesktop || window.onAllDesktops;

    // If the window isn't on the current desktop or on all desktops, we don't need to change it.
    if (!windowIsRelevant) {
        return;
    }

    if (window.output === primaryScreen) {
        window.onAllDesktops = false;
    } else {
        window.onAllDesktops = true;
    }

    // If we haven't already connected to the outputChanged signal for this window, do so now.
    if (!windowMap.has(window)) {
        windowMap.set(window, true);
        window.outputChanged.connect(() => {
            processWindow(window)
        });
        window.closed.connect(() => {
            windowMap.delete(window);
        });
    }
}

function processAllWindows() {
    workspace.windowList().forEach((window) => {
        if (!window.normalWindow) return;
        processWindow(window);
    });
}

function onScreensChanged() {
    // Update the primary screen.
    primaryScreen = workspace.screenOrder[0];
    // Reprocess all windows.
    processAllWindows();
}

function main() {
    // Process existing windows on startup.
    processAllWindows();

    // When the screens change…
    workspace.screensChanged.connect(onScreensChanged);
    workspace.screenOrderChanged.connect(onScreensChanged);

    // When a new window is added…
    workspace.windowAdded.connect((window) => {
        processWindow(window);
    });
}

main();
