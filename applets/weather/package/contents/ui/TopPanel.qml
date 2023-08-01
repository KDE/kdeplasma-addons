/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.9

import QtQuick.Layouts 1.3

import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.components 3.0 as PlasmaComponents

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

    columnSpacing: Kirigami.Units.gridUnit
    rowSpacing: Kirigami.Units.gridUnit

    columns: 3

    Kirigami.Heading {
        id: locationLabel

        Layout.row: 0
        Layout.column: 0
        Layout.columnSpan: 3
        Layout.fillWidth: true

        wrapMode: Text.NoWrap

        text: generalModel.location
    }

    Kirigami.Icon {
        id: conditionIcon

        Layout.row: 1
        Layout.column: 1
        Layout.minimumHeight: Kirigami.Units.iconSizes.huge
        Layout.minimumWidth: Kirigami.Units.iconSizes.huge
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

        KSvg.SvgItem {
            id: windSpeedDirection

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            Layout.preferredHeight: naturalSize.height
            Layout.preferredWidth: naturalSize.width

            imagePath: "weather/wind-arrows"
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

        Kirigami.Heading {
            id: tempLabel

            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

            level: 3
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.NoWrap

            text: observationModel.temperature
        }
    }
}
