import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.weatherdata as WeatherData

Item {
    id: root

    required property real min
    required property real max

    required property var formatter

    property bool leftAllign: false
    property bool bottomAllign: false

    property real spacing: 0

    property bool horizontal: false

    property real estimatedLabelWidth: Kirigami.Units.gridUnit

    property int labelsCount: 2

    onVisibleChanged: updateSize()

    Repeater {
        id: repeater
        model: root.labelsCount

        PlasmaComponents.Label {
            id: delegate
            required property int index

            property real value: {
                return root.min + index * (root.max - root.min) / (root.labelsCount - 1);
            }

            anchors.leftMargin: root.spacing
            anchors.rightMargin: root.spacing

            text: root.formatter(value)
            font: Kirigami.Theme.smallFont

            y: {
                if (root.horizontal) {
                    return 0;
                }

                const ratio = (value - root.min) / (root.max - root.min);

                return root.height - ratio * root.height - height / 2;
            }

            x: {
                if (!root.horizontal) {
                    return 0;
                }

                const ratio = (value - root.min) / (root.max - root.min);

                return ratio * root.width - implicitWidth / 2;
            }

            onTextChanged: root.updateSize()

            Component.onCompleted: {
                if (!root.horizontal) {
                    if (root.leftAllign) {
                        anchors.left = root.left;
                    } else {
                        anchors.right = root.right;
                    }
                } else {
                    if (root.bottomAllign) {
                        anchors.bottom = root.bottom;
                    } else {
                        anchors.top = root.top;
                    }
                }
                root.updateSize();
            }
        }
    }

    function updateSize() {
        if (root.horizontal) {
            root.updateHeight();
        } else {
            root.updateWidth();
        }
    }

    function updateWidth() {
        if (!root.visible) {
            root.implicitWidth = 0;
            return;
        }

        let maxWidth = 0;
        // Iterate through all generated child labels and find the width
        for (let i = 0; i < root.children.length; i++) {
            let child = root.children[i];
            if (child.implicitWidth > maxWidth) {
                maxWidth = child.implicitWidth;
            }
        }
        root.implicitWidth = maxWidth + root.spacing;
    }

    function updateHeight() {
        if (!root.visible) {
            root.implicitHeight = 0;
            return;
        }

        root.implicitHeight = root.children[0].implicitHeight + root.spacing;
    }
}
