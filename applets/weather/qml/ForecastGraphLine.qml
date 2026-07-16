import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.weatherdata as WeatherData

Item {
    id: root

    required property real graphMarginLeft
    required property real graphMarginRight
    required property real graphMarginBottom
    required property real graphMarginTop

    required property real graphVisualMinX
    required property real graphVisualMaxX

    required property color generalTempColor
    required property color highTempColor
    required property color lowTempColor
    required property color conditionProbabilityColor

    required property int invalidUnit
    required property int displayTemperatureUnit

    required property real graphHovered

    required property real currentPointDateX
    required property real currentPointGeneralTempY
    required property real currentPointHighTempY
    required property real currentPointLowTempY
    required property real currentPointConditionProbabilityY

    required property string generalTempText
    required property string highTempText
    required property string lowTempText
    required property string conditionProbabilityText

    property var maxTemp: null
    property var minTemp: null

    property bool hasProbability: false
    property bool highLowTempPresent: false

    readonly property real timestampLabelHeight: timestampLabel.height

    onCurrentPointDateXChanged: {
        Qt.callLater(() => {
            if (lineItem.x > (width / 2)) {
                if (state !== "leftLabels") {
                    state = "leftLabels";
                }
            } else {
                if (state !== "rightLabels") {
                    state = "rightLabels";
                }
            }
        });
    }

    PlasmaComponents.Label {
        id: timestampLabel
        anchors.top: parent.top
        anchors.horizontalCenter: lineItem.horizontalCenter
        visible: lineItem.visible
        text: {
            const timestamp = new Date(root.currentPointDateX);
            const format = Qt.locale().timeFormat(Locale.ShortFormat);
            const usesAmPm = format.includes("Ap");
            Qt.formatDateTime(timestamp, usesAmPm ? "h AP" : "HH:mm");
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
            width: Kirigami.Units.gridUnit * 0.1
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

        property real rectangleHeight: Kirigami.Units.gridUnit

        function calculateYFromTemp(temperature) {
            if (!root.minTemp || !root.maxTemp) {
                return 0;
            }
            let coefficient = 1.0 - temperature / 100;
            let graphPlaneHeight = dataLabels.height - root.graphMarginTop - root.graphMarginBottom;
            return coefficient * graphPlaneHeight + root.graphMarginTop - rectangleHeight / 2;
        }

        function updateLayout() {
            const spacing = Kirigami.Units.smallSpacing;
            const isRightAligned = (root.state === "rightLabels");

            let labels = [];

            for (const child of children) {
                // Skip non-visual children, connections, or components
                if (child.visible && child.hasOwnProperty("x")) {
                    labels.push(child);
                }
            }

            if (labels.length === 0)
                return;

            // Align labels to the left or right to prevent the spacing between the
            // line and labels
            for (const label of labels) {
                if (isRightAligned) {
                    // Anchor to the right edge
                    label.anchors.right = undefined;
                    label.anchors.left = dataLabels.left;
                } else {
                    // Anchor to the left edge
                    label.anchors.left = undefined;
                    label.anchors.right = dataLabels.right;
                }
            }

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

        Rectangle {
            implicitWidth: generalTempLabel.implicitWidth + Kirigami.Units.smallSpacing
            implicitHeight: dataLabels.rectangleHeight
            color: Qt.lighter(root.generalTempColor, 1.7)
            radius: Kirigami.Units.cornerRadius
            visible: !root.highLowTempPresent && root.generalTempText !== ""
            property real preferredY: dataLabels.calculateYFromTemp(root.currentPointGeneralTempY)
            PlasmaComponents.Label {
                id: generalTempLabel
                anchors.centerIn: parent
                text: root.generalTempText
                color: root.generalTempColor
            }
        }

        Rectangle {
            implicitWidth: highTempLabel.implicitWidth + Kirigami.Units.smallSpacing
            implicitHeight: dataLabels.rectangleHeight
            color: Qt.lighter(root.highTempColor, 1.7)
            radius: Kirigami.Units.cornerRadius
            property real preferredY: dataLabels.calculateYFromTemp(root.currentPointHighTempY)
            visible: root.highLowTempPresent && root.highTempText !== ""
            PlasmaComponents.Label {
                id: highTempLabel
                anchors.centerIn: parent
                text: root.highTempText
                color: root.highTempColor
            }
        }

        Rectangle {
            implicitWidth: lowTempLabel.implicitWidth + Kirigami.Units.smallSpacing
            implicitHeight: dataLabels.rectangleHeight
            color: Qt.lighter(root.lowTempColor, 1.7)
            radius: Kirigami.Units.cornerRadius
            visible: root.highLowTempPresent && root.lowTempText !== ""
            property real preferredY: dataLabels.calculateYFromTemp(root.currentPointLowTempY)
            PlasmaComponents.Label {
                id: lowTempLabel
                anchors.centerIn: parent
                text: root.lowTempText
                color: root.lowTempColor
            }
        }

        Rectangle {
            implicitWidth: conditionProbabilityLabel.implicitWidth + Kirigami.Units.smallSpacing
            implicitHeight: dataLabels.rectangleHeight
            color: Qt.lighter(root.conditionProbabilityColor, 1.7)
            radius: Kirigami.Units.cornerRadius
            visible: root.hasProbability && root.conditionProbabilityText !== ""
            property real preferredY: {
                let coefficient = (1.0 - root.currentPointConditionProbabilityY / 100);
                let graphPlaneHeight = dataLabels.height - root.graphMarginTop - root.graphMarginBottom;
                return coefficient * graphPlaneHeight + root.graphMarginTop - parent.rectangleHeight / 2;
            }
            PlasmaComponents.Label {
                id: conditionProbabilityLabel
                anchors.centerIn: parent
                text: root.conditionProbabilityText
                color: root.conditionProbabilityColor
            }
        }

        Connections {
            target: root

            function onCurrentPointDateXChanged() {
                Qt.callLater(() => {
                    dataLabels.updateLayout();
                });
            }
        }

        onVisibleChanged: {
            if (visible) {
                updateLayout();
            }
        }

        onHeightChanged: {
            if (visible) {
                updateLayout();
            }
        }
    }

    states: [
        State {
            name: "rightLabels"
            AnchorChanges {
                target: dataLabels
                anchors.left: lineItem.right
                anchors.right: undefined
            }

            PropertyChanges {
                dataLabels.anchors.leftMargin: Kirigami.Units.largeSpacing
                dataLabels.anchors.rightMargin: 0
            }
        },
        State {
            name: "leftLabels"
            AnchorChanges {
                target: dataLabels
                anchors.right: lineItem.left
                anchors.left: undefined
            }
            PropertyChanges {
                dataLabels.anchors.rightMargin: Kirigami.Units.largeSpacing
                dataLabels.anchors.leftMargin: 0
            }
        }
    ]
}
