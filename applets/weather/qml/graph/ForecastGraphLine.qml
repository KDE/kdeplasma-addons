/*
 * SPDX-FileCopyrightText: 2026 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

// The line and labels need to be positioned at arbitrary coordinates,
// so use Item here instead of Layout objects.
Item {
    id: root

    required property real graphMarginLeft
    required property real graphMarginRight
    required property real graphMarginBottom
    required property real graphMarginTop

    required property real graphVisualMinX
    required property real graphVisualMaxX

    required property int invalidUnit
    required property int displayTemperatureUnit

    required property real graphHovered

    required property int currentPointIndex
    required property real currentPointDateX
    required property var currentPointValues

    required property var seriesDefinitions

    required property var formatter

    property var maxTemp: null
    property var minTemp: null

    property bool hasProbability: false
    property bool highLowTempPresent: false

    readonly property real timestampLabelHeight: timestampLabel.height

    PlasmaComponents.Label {
        id: timestampLabel
        anchors.top: parent.top
        anchors.horizontalCenter: lineItem.horizontalCenter
        visible: lineItem.visible
        text: {
            const timestamp = new Date(root.currentPointDateX);
            return root.formatter(timestamp);
        }
    }

    Item {
        id: lineItem

        anchors.bottom: parent.bottom

        height: parent.height - timestampLabel.height

        x: {
            const plotWidth = parent.width - root.graphMarginLeft - root.graphMarginRight;

            if (plotWidth <= 0) {
                return root.graphMarginLeft;
            }

            if (root.graphVisualMaxX <= root.graphVisualMinX) {
                return root.graphMarginLeft;
            }

            const ratio = (root.currentPointDateX - root.graphVisualMinX) / (root.graphVisualMaxX - root.graphVisualMinX);

            let linePositionX = root.graphMarginLeft + ratio * plotWidth - cursorLine.width / 2;

            return linePositionX;
        }

        visible: root.graphHovered

        Rectangle {
            id: cursorLine
            width: 2
            height: parent.height
            color: Kirigami.Theme.highlightColor
        }
    }

    Item {
        id: dataLabels
        anchors.bottom: parent.bottom
        height: parent.height - timestampLabel.height
        width: childrenRect.width

        visible: lineItem.visible

        state: lineItem.x > root.width / 2 ? "leftLabels" : "rightLabels"

        property real rectangleHeight: Kirigami.Units.gridUnit

        function positionLabelsY() {
            const spacing = Kirigami.Units.smallSpacing;

            let labels = [];

            for (const child of children) {
                // Skip non-visual children, connections, or components
                if (child.visible && child.hasOwnProperty("preferredY")) {
                    labels.push(child);
                }
            }

            if (labels.length === 0)
                return;

            labels.sort((a, b) => a.preferredY - b.preferredY);

            // Initialize
            for (const label of labels) {
                label.y = label.preferredY;
            }

            // Forward pass: remove overlaps
            for (let i = 1; i < labels.length; ++i) {
                const previous = labels[i - 1];
                const current = labels[i];

                const minY = previous.y + previous.height + spacing;

                if (current.y < minY) {
                    current.y = minY;
                }
            }

            // Preserve the visual center
            const preferredCenter = (labels[0].preferredY + labels[labels.length - 1].preferredY) / 2;
            const actualCenter = (labels[0].y + labels[labels.length - 1].y) / 2;
            const centerShift = preferredCenter - actualCenter;

            for (const label of labels) {
                label.y += centerShift;
            }

            // Clamp to top boundary
            let shift = 0;
            if (labels[0].y < 0) {
                shift = -labels[0].y;
            }

            // Clamp to bottom boundary
            const last = labels[labels.length - 1];
            if (last.y + last.height > height) {
                shift = height - last.height - last.y;
            }

            if (shift !== 0) {
                for (const label of labels) {
                    label.y += shift;
                }
            }
        }

        Repeater {
            model: root.seriesDefinitions

            delegate: Kirigami.Badge {
                id: badge

                required property var modelData

                customColor: Qt.tint(Kirigami.Theme.backgroundColor, Qt.alpha(modelData.color, 0.15))

                customTextColor: modelData.color

                padding: 1

                visible: modelData.visible && !!modelData.labelTextFunc && text !== ""

                state: {
                    const labelsOnRight = dataLabels.state === "rightLabels";
                    const visualRight = mirrored ? !labelsOnRight : labelsOnRight;
                    return visualRight ? "right" : "left";
                }

                text: !!modelData.labelTextFunc ? modelData.labelTextFunc(root.currentPointIndex) : ""

                property real preferredY: {
                    const value = root.currentPointValues[modelData.name];

                    const graphPlaneHeight = dataLabels.height - root.graphMarginTop - root.graphMarginBottom;

                    const coefficient = 1.0 - value / 100;

                    return coefficient * graphPlaneHeight + root.graphMarginTop - height / 2;
                }

                states: [
                    State {
                        name: "right"
                        AnchorChanges {
                            target: badge
                            anchors.left: dataLabels.left
                            anchors.right: undefined
                        }
                    },
                    State {
                        name: "left"
                        AnchorChanges {
                            target: badge
                            anchors.left: undefined
                            anchors.right: dataLabels.right
                        }
                    }
                ]
            }
        }

        Connections {
            target: root

            function onCurrentPointDateXChanged() {
                Qt.callLater(() => {
                    dataLabels.positionLabelsY();
                });
            }
        }

        onVisibleChanged: {
            if (visible) {
                positionLabelsY();
            }
        }

        onHeightChanged: {
            if (visible) {
                positionLabelsY();
            }
        }

        states: [
            State {
                name: "rightLabels"
                AnchorChanges {
                    target: dataLabels
                    anchors.left: mirrored ? undefined : lineItem.right
                    anchors.right: mirrored ? lineItem.left : undefined
                }

                PropertyChanges {
                    dataLabels.anchors.leftMargin: mirrored ? 0 : Kirigami.Units.largeSpacing
                    dataLabels.anchors.rightMargin: mirrored ? Kirigami.Units.largeSpacing : 0
                }
            },
            State {
                name: "leftLabels"
                AnchorChanges {
                    target: dataLabels
                    anchors.right: mirrored ? undefined : lineItem.left
                    anchors.left: mirrored ? lineItem.right : undefined
                }
                PropertyChanges {
                    dataLabels.anchors.rightMargin: mirrored ? 0 : Kirigami.Units.largeSpacing
                    dataLabels.anchors.leftMargin: mirrored ? Kirigami.Units.largeSpacing : 0
                }
            }
        ]
    }
}
