/*
 * SPDX-FileCopyrightText: 2026 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QtGraphs

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

GridLayout {
    id: root

    required property var seriesDefinitions

    columnSpacing: Kirigami.Units.largeSpacing

    Repeater {
        model: {
            const forecastLegend = [];

            for (let i = 0; i < root.seriesDefinitions.length; i++) {
                const series = root.seriesDefinitions[i];

                // Only add the series to the legend if it is currently visible
                if (series.visible) {
                    const legendItem = {
                        label: series.legendText,
                        color: series.color
                    };
                    forecastLegend.push(legendItem);
                }
            }

            return forecastLegend;
        }

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
                font.family: Kirigami.Theme.smallFont.family
                font.pixelSize: Kirigami.Theme.smallFont.pixelSize
                color: Kirigami.Theme.disabledTextColor
            }
        }
    }
}
