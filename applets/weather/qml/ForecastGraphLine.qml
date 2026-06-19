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

    required property color generalTempColor
    required property color highTempColor
    required property color lowTempColor

    required property real pointSpacing

    property var maxTemp: null
    property var minTemp: null

    property var highTemp: null
    property var lowTemp: null
    property var generalTemp: null
    property var timestamp: null

    property bool highLowTempPresent: false

    readonly property bool hovered: hoverHandler.hovered

    readonly property real timestampLabelHeight: timestampLabel.height

    readonly property int pointNumber: {
        const graphPositionX = hoverHandler.point.position.x - root.graphMarginRight;
        if ((graphPositionX % root.pointSpacing) > (root.pointSpacing / 2) || hoverHandler.point.position.x < (root.pointSpacing / 2)) {
            return Math.ceil(graphPositionX / root.pointSpacing);
        } else {
            return Math.floor(graphPositionX / root.pointSpacing);
        }
    }

    onPointNumberChanged: {
        if (lineItem.x > (width / 2)) {
            if (state !== "leftLabels") {
                state = "leftLabels";
            }
        } else {
            if (state !== "rightLabels") {
                state = "rightLabels";
            }
        }
    }

    PlasmaComponents.Label {
        id: timestampLabel
        anchors.top: parent.top
        anchors.horizontalCenter: lineItem.horizontalCenter
        visible: lineItem.visible
        text: {
            if (!root.timestamp) {
                return "";
            }
            return root.timestamp;
        }
    }

    Item {
        id: lineItem

        anchors.bottom: parent.bottom

        height: parent.height - timestampLabel.height

        x: {
            return root.pointNumber * root.pointSpacing + root.graphMarginRight - cursorLine.width / 2;
        }

        visible: hoverHandler.hovered

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
            let coefficient = (1.0 - (temperature - root.minTemp) / (root.maxTemp - root.minTemp));
            let graphPlaneHeight = dataLabels.height - root.graphMarginTop - root.graphMarginBottom;
            return coefficient * graphPlaneHeight + root.graphMarginTop - rectangleHeight / 2;
        }

        function updateLayout() {
            const spacing = Kirigami.Units.smallSpacing;

            let labels = [];

            for (const child of children) {
                if (child.visible) {
                    labels.push(child);
                }
            }

            if (labels.length === 0)
                return;

            // Sort by preferred position
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
            visible: !!root.generalTemp && !root.highLowTempPresent
            property real preferredY: dataLabels.calculateYFromTemp(root.generalTemp ?? 0)
            PlasmaComponents.Label {
                id: generalTempLabel
                anchors.centerIn: parent
                text: root.generalTemp ?? ""
                color: root.generalTempColor
            }
        }

        Rectangle {
            implicitWidth: highTempLabel.implicitWidth + Kirigami.Units.smallSpacing
            implicitHeight: dataLabels.rectangleHeight
            color: Qt.lighter(root.highTempColor, 1.7)
            radius: Kirigami.Units.cornerRadius
            property real preferredY: dataLabels.calculateYFromTemp(root.highTemp ?? 0)
            visible: !!root.highTemp && root.highLowTempPresent
            PlasmaComponents.Label {
                id: highTempLabel
                anchors.centerIn: parent
                text: root.highTemp ?? ""
                color: root.highTempColor
            }
        }

        Rectangle {
            implicitWidth: lowTempLabel.implicitWidth + Kirigami.Units.smallSpacing
            implicitHeight: dataLabels.rectangleHeight
            color: Qt.lighter(root.lowTempColor, 1.7)
            radius: Kirigami.Units.cornerRadius
            visible: !!root.lowTemp && root.highLowTempPresent
            property real preferredY: dataLabels.calculateYFromTemp(root.lowTemp ?? 0)
            PlasmaComponents.Label {
                id: lowTempLabel
                anchors.centerIn: parent
                text: root.lowTemp ?? ""
                color: root.lowTempColor
            }
        }

        Connections {
            target: root

            function onPointNumberChanged() {
                dataLabels.updateLayout();
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

    HoverHandler {
        id: hoverHandler
    }
}
