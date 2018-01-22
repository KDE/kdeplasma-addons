/*
 * Copyright 2018  Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.9

import QtQuick.Layouts 1.3

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

PlasmaCore.FrameSvgItem {
    property var model

    implicitWidth: contentLayout.implicitWidth
    implicitHeight: contentLayout.implicitHeight

    Layout.minimumHeight: contentLayout.Layout.minimumHeight + margins.top + margins.bottom
    Layout.minimumWidth: contentLayout.Layout.minimumWidth + margins.left + margins.right
    Layout.preferredHeight: contentLayout.Layout.preferredHeight + margins.top + margins.bottom
    Layout.preferredWidth: contentLayout.Layout.preferredWidth + margins.left + margins.right
    Layout.maximumHeight: contentLayout.Layout.maximumHeight + margins.top + margins.bottom
    Layout.maximumWidth: contentLayout.Layout.maximumWidth + margins.left + margins.right

    visible: !!model.location
    imagePath: "widgets/frame"
    prefix: "plain"

    GridLayout {
        id: contentLayout

        anchors {
            fill: parent
            leftMargin: parent.margins.left
            topMargin: parent.margins.top
            rightMargin: parent.margins.right
            bottomMargin: parent.margins.bottom
        }
        Layout.preferredWidth: iconItem.Layout.preferredWidth + locationLabel.Layout.preferredWidth + tempLabel.Layout.preferredWidth + 2 * columnSpacing

        columnSpacing: units.largeSpacing
        rowSpacing: units.smallSpacing

        columns: 3
        rows: 2

        PlasmaCore.IconItem {
            id: iconItem

            Layout.row: 0
            Layout.column: 0
            Layout.rowSpan: 2
            Layout.preferredHeight: units.iconSizes.huge
            Layout.preferredWidth: units.iconSizes.huge

            source: model.currentConditionIcon
        }

        PlasmaExtras.Heading {
            id: locationLabel

            Layout.row: 0
            Layout.column: 1
            Layout.fillWidth: true
            Layout.preferredWidth: implicitWidth

            level: 2
            elide: Text.ElideRight
            wrapMode: Text.NoWrap

            text: model.location

            PlasmaCore.ToolTipArea {
                id: locationToolTip

                anchors.fill: parent
                visible: parent.truncated
                mainText: parent.text
            }
        }

        PlasmaComponents.Label {
            id: conditionLabel

            Layout.row: 1
            Layout.column: 1
            Layout.preferredWidth: implicitWidth

            text: model.currentConditions
        }

        PlasmaExtras.Heading {
            id: tempLabel

            Layout.row: 0
            Layout.column: 2
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            Layout.preferredWidth: implicitWidth

            level: 2
            wrapMode: Text.NoWrap

            text: model.currentTemperature
        }

        PlasmaComponents.Label {
            id: forecastTempsLabel

            Layout.row: 1
            Layout.column: 2
            Layout.alignment: Qt.AlignBaseline | Qt.AlignRight
            Layout.preferredWidth: implicitWidth

            text: {
                var low = model.currentDayLowTemperature, high = model.currentDayHighTemperature;
                if (!!low && !!high) {
                    return i18nc("High & Low temperature", "H: %1 L: %2", high, low);
                }
                if (!!low) {
                    return i18nc("Low temperature", "Low: %1", low);
                }
                if (!!high) {
                    return i18nc("High temperature", "High: %1", high);
                }
                return "";
            }
        }
    }
}
