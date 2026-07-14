import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QtGraphs

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

GridLayout {
    id: root

    required property var forecastLegendData

    columnSpacing: Kirigami.Units.largeSpacing

    Repeater {
        model: root.forecastLegendData
        RowLayout {
            id: legendDelegate
            required property var modelData

            spacing: Kirigami.Units.largeSpacing

            Rectangle {
                id: legendRectangle
                Layout.preferredHeight: legendLabel.height
                Layout.preferredWidth: height
                color: legendDelegate.modelData.color
            }

            PlasmaComponents.Label {
                id: legendLabel
                text: legendDelegate.modelData.label
            }
        }
    }
}
