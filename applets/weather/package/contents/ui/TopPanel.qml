/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

    columnSpacing: PlasmaCore.Units.largeSpacing
    rowSpacing: PlasmaCore.Units.smallSpacing

    columns: 3

    PlasmaCore.Svg {
        id: windSvg

        imagePath: "weather/wind-arrows"
        colorGroup: parent.PlasmaCore.ColorScope.colorGroup
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
        Layout.minimumHeight: PlasmaCore.Units.iconSizes.huge
        Layout.minimumWidth: PlasmaCore.Units.iconSizes.huge
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

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
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
