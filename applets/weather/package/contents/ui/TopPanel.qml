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

GridLayout {
    property var generalModel
    property var observationModel

    readonly property int sideWidth: Math.max(
        windSpeedLabel.implicitWidth,
        tempLabel.implicitWidth,
        windSpeedDirection.naturalSize.width
    )

    Layout.minimumWidth: Math.max(
        locationLabel.implicitWidth,
        (sideWidth + columnSpacing) * 2 + conditionIcon.Layout.minimumWidth
    )

    visible: !!generalModel.location

    columnSpacing: units.largeSpacing
    rowSpacing: units.smallSpacing

    columns: 3

    PlasmaCore.Svg {
        id: windSvg

        imagePath: "weather/wind-arrows"
    }

    PlasmaExtras.Heading {
        id: locationLabel

        Layout.row: 0
        Layout.column: 0
        Layout.columnSpan: 3
        Layout.fillWidth: true

        wrapMode: Text.NoWrap

        text: generalModel.location
    }

    PlasmaCore.IconItem {
        id: conditionIcon

        Layout.row: 1
        Layout.column: 1
        Layout.minimumHeight: units.iconSizes.huge
        Layout.minimumWidth: units.iconSizes.huge
        Layout.preferredHeight: Layout.minimumHeight
        Layout.preferredWidth: Layout.minimumWidth
        Layout.fillWidth: true

        source: generalModel.currentConditionIconName
    }

    PlasmaComponents.Label {
        id: conditionLabel

        Layout.row: 2
        Layout.column: 0
        Layout.columnSpan: 3
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

        text: observationModel.conditions
    }

    ColumnLayout {
        Layout.row: 1
        Layout.column: 0
        Layout.minimumWidth: sideWidth
        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

        PlasmaCore.SvgItem {
            id: windSpeedDirection

            Layout.preferredHeight: naturalSize.height
            Layout.preferredWidth: naturalSize.width

            svg: windSvg
            elementId: observationModel.windDirectionId || ""

            visible: !!observationModel.windDirectionId
        }

        PlasmaComponents.Label {
            id: windSpeedLabel

            text: observationModel.windSpeed
        }
    }

    ColumnLayout {
        Layout.row: 1
        Layout.column: 2
        Layout.minimumWidth: sideWidth
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

        PlasmaExtras.Heading {
            id: tempLabel

            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

            level: 3
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.NoWrap

            text: observationModel.temperature
        }
    }
}
