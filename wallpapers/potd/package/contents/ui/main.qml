/*
 *   SPDX-FileCopyrightText: 2016 Weng Xuetian <wengxt@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Window 2.15

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.wallpapers.potd 1.0

QQC2.StackView {
    id: root

    readonly property int fillMode: wallpaper.configuration.FillMode
    readonly property size sourceSize: Qt.size(root.width * Screen.devicePixelRatio, root.height * Screen.devicePixelRatio)
    property Item pendingImage
    property bool doesSkipAnimation: true

    onFillModeChanged: Qt.callLater(root.loadImage)
    onSourceSizeChanged: Qt.callLater(root.loadImage)

    function loadImage() {
        if (backend.localUrl.length === 0) {
            return;
        }
        if (root.pendingImage) {
            root.pendingImage.statusChanged.disconnect(replaceWhenLoaded);
            root.pendingImage.destroy();
            root.pendingImage = null;
        }

        root.doesSkipAnimation = root.empty
        root.pendingImage = imageComponent.createObject(root, {
            "source": backend.localUrl,
            "fillMode": root.fillMode,
            "opacity": root.doesSkipAnimation ? 1 : 0,
            "sourceSize": root.sourceSize,
            "width": root.width,
            "height": root.height,
        });
        root.pendingImage.statusChanged.connect(root.replaceWhenLoaded);
        root.replaceWhenLoaded();
    }

    function replaceWhenLoaded() {
        if (root.pendingImage.status === Image.Loading) {
            return;
        }
        root.pendingImage.statusChanged.disconnect(root.replaceWhenLoaded);
        root.replace(root.pendingImage, {}, root.doesSkipAnimation ? QQC2.StackView.Immediate : QQC2.StackView.Transition);
        root.pendingImage = null;
    }

    PotdBackend {
        id: backend
        identifier: wallpaper.configuration.Provider
        arguments: {
            if (identifier === "unsplash") {
                // Needs to specify category for unsplash provider
                return [wallpaper.configuration.Category];
            } else if (identifier === "bing") {
                // Bing supports 1366/1920/UHD resolutions
                const w = Screen.width * Screen.devicePixelRatio > 1920 ? 3840 : 1920;
                const h = Screen.height * Screen.devicePixelRatio > 1080 ? 2160 : 1080;
                return [w, h];
            }
            return [];
        }
        updateOverMeteredConnection: wallpaper.configuration.UpdateOverMeteredConnection

        onImageChanged: Qt.callLater(root.loadImage)
        onLocalUrlChanged: Qt.callLater(root.loadImage)
    }

    Component {
        id: imageComponent

        Image {
            asynchronous: true
            cache: false
            autoTransform: true
            smooth: true

            QQC2.StackView.onActivated: wallpaper.repaintNeeded()
            QQC2.StackView.onRemoved: destroy()
        }
    }

    Rectangle {
        id: backgroundColor
        anchors.fill: parent
        color: wallpaper.configuration.Color
        Behavior on color {
            ColorAnimation { duration: PlasmaCore.Units.longDuration }
        }
    }

    replaceEnter: Transition {
        OpacityAnimator {
            id: replaceEnterOpacityAnimator
            from: 0
            to: 1
            // As the wallpaper is updated once a day, the transition should be longer.
            duration: Math.round(PlasmaCore.Units.veryLongDuration * 5)
        }
    }
    // Keep the old image around till the new one is fully faded in
    // If we fade both at the same time you can see the background behind glimpse through
    replaceExit: Transition {
        PauseAnimation {
            duration: replaceEnterOpacityAnimator.duration
        }
    }
}
