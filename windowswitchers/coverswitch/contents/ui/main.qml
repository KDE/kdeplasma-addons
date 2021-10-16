/*
 SPDX-FileCopyrightText: 2021 Ismael Asensio <isma.af@gmail.com>

 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3

import org.kde.kwin 2.0 as KWin


KWin.Switcher {
    id: tabBox
    currentIndex: thumbnailView ? thumbnailView.currentIndex : -1

    PlasmaCore.Dialog {
        id: dialog
        location: PlasmaCore.Types.Floating
        visible: tabBox.visible
        flags: Qt.X11BypassWindowManagerHint
        x: screenGeometry.x
        y: screenGeometry.y

        mainItem: ColumnLayout {
            width: tabBox.screenGeometry.width - dialog.margins.left - dialog.margins.right
            height: tabBox.screenGeometry.height - dialog.margins.top - dialog.margins.bottom

            PathView {
                id: thumbnailView

                readonly property int visibleCount: Math.min(count, pathItemCount)

                readonly property int boxWidth: tabBox.screenGeometry.width / 2
                readonly property int boxHeight: tabBox.screenGeometry.height / 2

                focus: true
                Layout.fillWidth: true
                Layout.fillHeight: true

                preferredHighlightBegin: 0.49
                preferredHighlightEnd: 0.49
                highlightRangeMode: PathView.StrictlyEnforceRange

                // This property sets the animation duration between the current position to the next one,
                // without taking into account how much distance the thumbnails travel in that time.
                // To compensate the speed, we slowly reduce the duration with the number of thumbnails,
                // starting from `veryLongDuration` when there are 2 of them
                highlightMoveDuration: PlasmaCore.Units.veryLongDuration * (2 / Math.sqrt(visibleCount + 1))

                pathItemCount: 13

                path: Path {
                    // Left stack
                    startX: thumbnailView.width * 0.1; startY: thumbnailView.height * 0.55
                    PathAttribute { name: "progress"; value: 0 }
                    PathAttribute { name: "scale"; value: 0.7 }
                    PathAttribute { name: "rotation"; value: 70 }
                    PathPercent { value: 0 }

                    PathLine { x: thumbnailView.width * 0.25 ; y: thumbnailView.height * 0.55 }
                    PathAttribute { name: "progress"; value: 0.8 }
                    PathAttribute { name: "scale"; value: 0.7 }
                    PathAttribute { name: "rotation"; value: 70 }
                    PathPercent { value: 0.4 }

                    // Center Item
                    PathQuad {
                        x: thumbnailView.width * 0.5 ; y: thumbnailView.height * 0.65
                        controlX: thumbnailView.width * 0.45; controlY: thumbnailView.height * 0.6
                    }
                    PathAttribute { name: "progress"; value: 1 }
                    PathAttribute { name: "scale"; value: 1 }
                    PathAttribute { name: "rotation"; value: 0 }
                    PathPercent { value: 0.49 } // A bit less than 50% so items preferrably stack on the right side

                    // Right stack
                    PathQuad {
                        x: thumbnailView.width * 0.75 ; y: thumbnailView.height * 0.55
                        controlX: thumbnailView.width * 0.55; controlY: thumbnailView.height * 0.6
                    }
                    PathAttribute { name: "progress"; value: 0.8 }
                    PathAttribute { name: "scale"; value: 0.7 }
                    PathAttribute { name: "rotation"; value: -70 }
                    PathPercent { value: 0.6 }

                    PathLine { x: thumbnailView.width * 0.9 ; y: thumbnailView.height * 0.55 }
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

                    width: Math.round(thumbnail.implicitWidth * scaleFactor)
                    height: Math.round(thumbnail.implicitHeight * scaleFactor)
                    scale: PathView.onPath ? PathView.scale : 0
                    z: PathView.onPath ? Math.floor(PathView.progress * thumbnailView.visibleCount) : -1

                    KWin.ThumbnailItem {
                        id: thumbnail
                        readonly property double ratio: implicitWidth / implicitHeight

                        wId: windowId
                        anchors.fill: parent
                    }

                    Kirigami.ShadowedRectangle {
                        anchors.fill: parent
                        z: -1

                        color: "transparent"
                        shadow.size: PlasmaCore.Units.gridUnit
                        shadow.color: "black"
                        opacity: 0.5
                    }

                    transform: Rotation {
                        origin { x: delegateItem.width/2; y: delegateItem.height/2 }
                        axis { x: 0; y: 1; z: 0 }
                        angle: delegateItem.PathView.rotation
                    }

                    TapHandler {
                        grabPermissions: PointerHandler.TakeOverForbidden
                        gesturePolicy: TapHandler.WithinBounds
                        onSingleTapped: {
                            if (index === thumbnailView.currentIndex) {
                                thumbnailView.model.activate(index);
                                return;
                            }
                            thumbnailView.movementDirection = (delegateItem.PathView.rotation < 0) ? PathView.Positive : PathView.Negative
                            thumbnailView.currentIndex = index
                        }
                    }
                }

                highlight: PlasmaCore.FrameSvgItem {
                    id: highlightItem
                    imagePath: "widgets/viewitem"
                    prefix: "hover"

                    readonly property Item target: thumbnailView.currentItem

                    visible: target !== null
                    // Make sure the highlight is pixel perfect aligned on both sides even if the target is not
                    anchors.centerIn: target
                    anchors.horizontalCenterOffset: target ? Math.round(target.x) - target.x : 0
                    anchors.verticalCenterOffset: target ? Math.round(target.y) - target.y : 0
                    width: target ? Math.round(target.width/2 + 3 * PlasmaCore.Units.smallSpacing) * 2 : 0
                    height: target ? Math.round(target.height/2 + 3 * PlasmaCore.Units.smallSpacing) * 2 : 0
                    scale: target ? target.scale : 1
                    z: target ? target.z - 0.5 : -0.5
                    // The transform cannot be directly assigned as the transform origin is different
                    transform: Rotation {
                        origin { x: highlightItem.width/2; y: highlightItem.height/2 }
                        axis { x: 0; y: 1; z: 0 }
                        angle: target ? target.PathView.rotation : 0
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
                Layout.preferredHeight: PlasmaCore.Units.iconSizes.large
                Layout.margins: PlasmaCore.Units.gridUnit
                Layout.alignment: Qt.AlignCenter
                spacing: PlasmaCore.Units.gridUnit

                PlasmaCore.IconItem {
                    source: thumbnailView.currentItem ? thumbnailView.currentItem.icon : ""
                    implicitWidth: PlasmaCore.Units.iconSizes.large
                    implicitHeight: PlasmaCore.Units.iconSizes.large
                    Layout.alignment: Qt.AlignCenter
                }

                PC3.Label {
                    font.bold: true
                    font.pointSize: Math.round(PlasmaCore.Theme.defaultFont.pointSize * 1.6)
                    text: thumbnailView.currentItem ? thumbnailView.currentItem.caption : ""
                    maximumLineCount: 1
                    elide: Text.ElideMiddle
                    Layout.maximumWidth: tabBox.screenGeometry.width * 0.8
                    Layout.alignment: Qt.AlignCenter
                }
            }
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
        } else if (currentIndex === (thumbnailView.count - 1) && thumbnailView.currentIndex === 0) {
            thumbnailView.movementDirection = PathView.Negative
        } else {
            thumbnailView.movementDirection = (currentIndex > thumbnailView.currentIndex) ? PathView.Positive : PathView.Negative
        }

        thumbnailView.currentIndex = tabBox.currentIndex
    }

    onVisibleChanged: {
        // Reset the PathView index when hiding to avoid unwanted animations on relaunch
        if (!visible) {
            thumbnailView.currentIndex = 0;
        }
    }
}
