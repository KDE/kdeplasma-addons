/*
 SPDX-FileCopyrightText: 2021-2023 Ismael Asensio <isma.af@gmail.com>

 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import Qt5Compat.GraphicalEffects
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.components 3.0 as PC3

import org.kde.kwin 3.0 as KWin
import org.kde.kwin.private.effects 1.0


KWin.TabBoxSwitcher {
    id: tabBox
    currentIndex: thumbnailView ? thumbnailView.currentIndex : -1

    // TODO: Make it user configurable ?
    property bool enableBlur: true

    property bool originalState: true
    property var currentDesktop: KWin.Workspace.currentDesktop
    // Keep track of VD changes while the switcher is not visible
    Connections {
        target: KWin.Workspace
        function onCurrentDesktopChanged() {
            tabBox.currentDesktop = KWin.Workspace.currentDesktop
        }
    }

    automaticallyHide: false

    component IntroAnimation : NumberAnimation {
        duration: Kirigami.Units.veryLongDuration
        easing.type: Easing.InOutCubic
    }

    Window {
        id: window

        x: tabBox.screenGeometry.x
        y: tabBox.screenGeometry.y
        width: tabBox.screenGeometry.width
        height: tabBox.screenGeometry.height
        flags: Qt.BypassWindowManagerHint | Qt.FramelessWindowHint
        visibility: Window.FullScreen
        // Workaround QTBUG-35244. Do not directly assign here to avoid warning
        visible: true

        color: "transparent"

        KWin.DesktopBackground {
            activity: KWin.Workspace.currentActivity
            desktop: KWin.Workspace.currentVirtualDesktop
            outputName: window.screen.name

            layer.enabled: true
            layer.effect: FastBlur {
                radius: (enableBlur && !originalState) ? 64 : 0
                Behavior on radius { IntroAnimation {} }
            }
        }

        Rectangle {
            anchors {
                top: enableBlur ? parent.top : infoBar.top
                topMargin: enableBlur ? 0 : -infoBar.anchors.bottomMargin
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            color: Kirigami.Theme.backgroundColor
            opacity: originalState ? 0 : enableBlur ? 0.5 : 0.75
            Behavior on opacity { IntroAnimation {} }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.largeSpacing * 2
            icon.source: "edit-none"
            text: i18ndc("kwin", "@info:placeholder no entries in the task switcher", "No open windows")
            visible: thumbnailView.count === 0
        }


        PathView {
            id: thumbnailView

            readonly property int visibleCount: Math.min(count, pathItemCount)
            readonly property real boxScaleFactor: 0.5
            readonly property int boxWidth: tabBox.screenGeometry.width * boxScaleFactor
            readonly property int boxHeight: tabBox.screenGeometry.height * boxScaleFactor

            focus: true

            anchors.fill: parent

            preferredHighlightBegin: 0.49
            preferredHighlightEnd: preferredHighlightBegin
            highlightRangeMode: PathView.StrictlyEnforceRange

            // This property sets the animation duration between the current position to the next one,
            // without taking into account how much distance the thumbnails travel in that time.
            // To compensate the speed, we slowly reduce the duration with the number of thumbnails,
            // starting from `veryLongDuration` when there are 2 of them
            highlightMoveDuration: Kirigami.Units.veryLongDuration * (2 / Math.sqrt(visibleCount + 1))

            pathItemCount: 13

            path: Path {
                // Left stack
                startX: thumbnailView.width * 0.1; startY: thumbnailView.height * 0.5
                PathAttribute { name: "progress"; value: 0 }
                PathAttribute { name: "scale"; value: 0.7 }
                PathAttribute { name: "rotation"; value: 70 }
                PathPercent { value: 0 }

                PathLine { x: thumbnailView.width * 0.25 ; y: thumbnailView.height * 0.5 }
                PathAttribute { name: "progress"; value: 0.8 }
                PathAttribute { name: "scale"; value: 0.7 }
                PathAttribute { name: "rotation"; value: 70 }
                PathPercent { value: 0.4 }

                // Center Item
                PathQuad {
                    x: thumbnailView.width * 0.5 ; y: thumbnailView.height * 0.6
                    controlX: thumbnailView.width * 0.45; controlY: thumbnailView.height * 0.55
                }
                PathAttribute { name: "progress"; value: 1 }
                PathAttribute { name: "scale"; value: 1 }
                PathAttribute { name: "rotation"; value: 0 }
                PathPercent { value: 0.49 } // A bit less than 50% so items preferrably stack on the right side

                // Right stack
                PathQuad {
                    x: thumbnailView.width * 0.75 ; y: thumbnailView.height * 0.5
                    controlX: thumbnailView.width * 0.55; controlY: thumbnailView.height * 0.55
                }
                PathAttribute { name: "progress"; value: 0.8 }
                PathAttribute { name: "scale"; value: 0.7 }
                PathAttribute { name: "rotation"; value: -70 }
                PathPercent { value: 0.6 }

                PathLine { x: thumbnailView.width * 0.9 ; y: thumbnailView.height * 0.5 }
                PathAttribute { name: "progress"; value: 0 }
                PathAttribute { name: "scale"; value: 0.7 }
                PathAttribute { name: "rotation"; value: -70 }
                PathPercent { value: 1 }
            }

            model: tabBox.model

            delegate: Item {
                id: delegateItem

                readonly property string caption: model.caption
                readonly property var icon: model.icon
                readonly property var desktops: thumbnail.client ? thumbnail.client.desktops : []

                readonly property real scaleFactor: {
                    if (thumbnail.implicitWidth < thumbnailView.boxWidth && thumbnail.implicitHeight < thumbnailView.boxHeight) {
                        // Do not scale up thumbnails smaller than the box frame
                        return 1;
                    } else if (thumbnail.ratio > thumbnailView.boxWidth / thumbnailView.boxHeight) {
                        // Thumbnail is wider than the box
                        return thumbnailView.boxWidth / thumbnail.implicitWidth;
                    } else {
                        // Thumbnail is taller than the box
                        return thumbnailView.boxHeight / thumbnail.implicitHeight;
                    }
                }

                property real rotationAngle: 0

                width: thumbnail.implicitWidth
                height: thumbnail.implicitHeight

                KWin.WindowThumbnail {
                    id: thumbnail

                    readonly property double ratio: implicitWidth / implicitHeight

                    wId: windowId
                    width: implicitWidth
                    height: implicitHeight

                    Kirigami.ShadowedRectangle {
                        anchors.fill: parent
                        z: -1

                        color: "transparent"
                        shadow.size: Kirigami.Units.gridUnit
                        shadow.color: "black"
                        opacity: 0.5
                    }
                }

                transform: Rotation {
                    origin { x: delegateItem.width/2; y: delegateItem.height/2 }
                    axis { x: 0; y: 1; z: 0 }
                    angle: delegateItem.rotationAngle
                }

                TapHandler {
                    enabled: !originalState
                    grabPermissions: PointerHandler.TakeOverForbidden
                    gesturePolicy: TapHandler.WithinBounds
                    onSingleTapped: {
                        if (index === thumbnailView.currentIndex) {
                            thumbnailView.model.activate(index);
                            return;
                        }
                        thumbnailView.movementDirection = (delegateItem.PathView.rotation < 0) ? PathView.Positive : PathView.Negative;
                        thumbnailView.currentIndex = index;
                    }
                }

                state: {
                    if (!originalState) return "onPath";
                    if (thumbnail.client.minimized || thumbnail.client.hidden) return "hidden";
                    if (desktops.length === 0 || desktops.includes(tabBox.currentDesktop)) return "clientPosition";
                    return "hidden";
                }

                states: [
                    State {
                        name: "onPath"
                        PropertyChanges {
                            target: delegateItem
                            z: PathView.onPath ? Math.floor(PathView.progress * thumbnailView.visibleCount) : -1
                            scale: PathView.scale * scaleFactor
                            rotationAngle: PathView.rotation
                            opacity: 1
                        }
                        PropertyChanges {
                            target: thumbnail
                            x: 0
                            y: 0
                        }
                    }
                    ,
                    State {
                        name: "clientPosition"
                        PropertyChanges {
                            target: delegateItem
                            z: thumbnail.client.stackingOrder + (PathView.isCurrentItem ? thumbnailView.count : 0)
                            scale: 1
                            rotationAngle: 0
                            opacity: 1
                        }
                        PropertyChanges {
                            target: thumbnail
                            x: (client.x - tabBox.screenGeometry.x) - delegateItem.x
                            y: (client.y - tabBox.screenGeometry.y) - delegateItem.y
                        }
                    }
                    ,
                    State {
                        name: "hidden"
                        extend: "onPath"
                        PropertyChanges {
                            target: delegateItem
                            opacity: 0
                        }
                    }
                ]

                transitions: Transition {
                    IntroAnimation { properties: "x, y, z, scale, opacity, rotationAngle" }
                }
            }

            highlight: KSvg.FrameSvgItem {
                id: highlightItem
                imagePath: "widgets/viewitem"
                prefix: "hover"

                readonly property Item target: thumbnailView.currentItem

                visible: target !== null
                // Make sure the highlight is pixel perfect aligned on both sides even if the target is not
                anchors.centerIn: target
                anchors.horizontalCenterOffset: target ? Math.round(target.x) - target.x : 0
                anchors.verticalCenterOffset: target ? Math.round(target.y) - target.y : 0
                width: target ? Math.round(target.width * target.scaleFactor / 2 + 3 * Kirigami.Units.smallSpacing) * 2 : 0
                height: target ? Math.round(target.height * target.scaleFactor / 2 + 3 * Kirigami.Units.smallSpacing) * 2 : 0
                scale: target ? target.PathView.scale : 1
                z: target ? target.z - 0.5 : -0.5
                // The transform cannot be directly assigned as the transform origin is different
                transform: Rotation {
                    origin { x: highlightItem.width/2; y: highlightItem.height/2 }
                    axis { x: 0; y: 1; z: 0 }
                    angle: target ? target.PathView.rotation : 0
                }

                opacity: originalState ? 0 : 1
                Behavior on opacity {
                    SequentialAnimation {
                        PauseAnimation { duration: originalState ? Kirigami.Units.veryLongDuration : 0}
                        NumberAnimation { duration: Kirigami.Units.shortDuration }
                        PauseAnimation { duration: originalState ? 0 : Kirigami.Units.veryLongDuration }
                    }
                }
            }

            layer.enabled: true
            layer.smooth: true

            onMovementStarted: movementDirection = PathView.Shortest

            Keys.onUpPressed: decrementCurrentIndex()
            Keys.onLeftPressed: decrementCurrentIndex()
            Keys.onDownPressed: incrementCurrentIndex()
            Keys.onRightPressed: incrementCurrentIndex()
        }

        RowLayout {
            id: infoBar

            height: Kirigami.Units.iconSizes.large
            spacing: Kirigami.Units.gridUnit
            visible: thumbnailView.count > 0

            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                margins: Kirigami.Units.gridUnit
            }

            Kirigami.Icon {
                source: thumbnailView.currentItem?.icon ?? ""
                implicitWidth: Kirigami.Units.iconSizes.large
                implicitHeight: Kirigami.Units.iconSizes.large
                Layout.alignment: Qt.AlignCenter
            }

            PC3.Label {
                font.bold: true
                font.pointSize: Math.round(Kirigami.Theme.defaultFont.pointSize * 1.6)
                text: thumbnailView.currentItem?.caption ?? ""
                textFormat: Text.PlainText
                maximumLineCount: 1
                elide: Text.ElideMiddle
                Layout.maximumWidth: tabBox.screenGeometry.width * 0.8
                Layout.alignment: Qt.AlignCenter
            }

            opacity: originalState ? 0 : 1
            Behavior on opacity { IntroAnimation {} }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.largeSpacing * 2
            icon.source: "edit-none"
            text: i18ndc("kwin", "@info:placeholder no entries in the task switcher", "No open windows")
            visible: thumbnailView.count === 0
        }
    }

    Timer {
        id: hidingTimer
        interval: Kirigami.Units.veryLongDuration
        onTriggered: {
            visible = false
        }
    }

    onCurrentIndexChanged: {
        if (currentIndex === thumbnailView.currentIndex) {
            return
        }

        // WindowSwitcher always changes currentIndex in increments of 1.
        // Detect the change direction and set the PathView movement accordingly, so fast changes
        // in the same direction don't result into a combination of forward and backward movements.
        if (thumbnailView.count === 2 || (currentIndex === 0 && thumbnailView.currentIndex === thumbnailView.count - 1)) {
            thumbnailView.movementDirection = PathView.Positive
        } else if (currentIndex === thumbnailView.count - 1 && thumbnailView.currentIndex === 0) {
            thumbnailView.movementDirection = PathView.Negative
        } else {
            thumbnailView.movementDirection = (currentIndex > thumbnailView.currentIndex) ? PathView.Positive : PathView.Negative
        }

        thumbnailView.currentIndex = tabBox.currentIndex
    }

    onVisibleChanged: {
        window.visible = visible
    }

    onAboutToShow: {
        hidingTimer.stop()
        tabBox.currentDesktop = KWin.Workspace.currentDesktop
        thumbnailView.positionViewAtIndex(tabBox.currentIndex, PathView.SnapPosition)
        Qt.callLater(() => originalState = false) // Delayed call to ensure that PathView delegates are correctly placed
    }

    onAboutToHide: {
        if (thumbnailView.currentItem
                && thumbnailView.currentItem.desktops.length > 0  // not on all desktops
                && !thumbnailView.currentItem.desktops.includes(tabBox.currentDesktop)) {
            // Select the latest VD the window is on, to mimic Workspace.activate()
            tabBox.currentDesktop = thumbnailView.currentItem.desktops.slice(-1)[0]
        }
        thumbnailView.positionViewAtIndex(tabBox.currentIndex, PathView.SnapPosition)
        Qt.callLater(() => originalState = true)
        hidingTimer.start()
    }
}
