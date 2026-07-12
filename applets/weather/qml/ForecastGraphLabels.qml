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

    readonly property int count: Math.floor((height - anchors.topMargin - anchors.bottomMargin) / Kirigami.Units.gridUnit)

    property bool leftAllign: false

    property real spacing: 0

    property real maxWidth: 0

    onVisibleChanged: {
        updateWidth();
    }

    Repeater {
        id: repeater
        model: root.count

        PlasmaComponents.Label {
            id: delegate
            required property int index

            property real value: {
                return root.min + index * (root.max - root.min) / (root.count - 1);
            }

            anchors.leftMargin: root.spacing
            anchors.rightMargin: root.spacing

            text: root.formatter(value)
            font: Kirigami.Theme.smallFont

            y: {
                const ratio = (value - root.min) / (root.max - root.min);

                return root.height - ratio * root.height - height / 2;
            }

            onTextChanged: {
                root.updateWidth();
            }

            Component.onCompleted: {
                if (root.leftAllign) {
                    anchors.left = root.left;
                } else {
                    anchors.right = root.right;
                }
                root.updateWidth();
            }
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
                maxWidth = child.contentWidth;
            }
        }
        root.implicitWidth = maxWidth + root.spacing;
    }
}
