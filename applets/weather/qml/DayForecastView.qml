/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

Item {
    id: root

    property var futureDays: null
    property var metaData: null

    property int displayTemperatureUnit: 0

    readonly property int preferredIconSize: Kirigami.Units.iconSizes.large

    implicitHeight: forecast.implicitHeight + horizontalHeader.height
    implicitWidth: forecast.implicitWidth + verticalHeader.width

    //Item to get the metrics of the regular font in a PlasmaComponent.Label
    PlasmaComponents.Label {
        id: helperLabel
        visible: false

        TextMetrics {
            id: labelFontMetrics
            text: "99%" // We want the sizing for the regular font, not emoji
            font: helperLabel.font // Explicitly use the actual Label's font even if it's the default one
        }
    }

    HorizontalHeaderView {
        id: horizontalHeader
        anchors.left: forecast.left
        anchors.top: parent.top
        syncView: (root.futureDays.daysNumber > 1) ? forecast : null
        clip: true
        textRole: "monthDay"
        resizableColumns: false
        interactive: false

        delegate: PlasmaComponents.Label {
            text: (!!model.monthDay) ? model.monthDay :
                  (!!model.weekDay) ? model.weekDay :
                  ""
            textFormat: Text.PlainText
            horizontalAlignment: Text.AlignHCenter
        }
    }

    VerticalHeaderView {
        id: verticalHeader
        anchors.top: forecast.top
        anchors.left: parent.left
        syncView: root.futureDays.isNightPresent ? forecast : null
        clip: true
        textRole: "period"
        resizableRows: false
        interactive: false

        delegate: PlasmaComponents.Label {
            text: !!model.period ? model.period : ""
            textFormat: Text.PlainText
        }
    }

    TableView {
        id: forecast

        interactive: false

        anchors.left: verticalHeader.right
        anchors.top: horizontalHeader.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        model: root.futureDays

        // calculate spacing and implicit width/height when layout changed
        onLayoutChanged: {
            //check if row loaded before calculating row height to prevent rows from being shown incorrectly
            if(isRowLoaded(topRow)) {
                var rowsHeight = implicitRowHeight(topRow) * rows;
                neededRowSpacing = (parent.height - horizontalHeader.height - rowsHeight) / (rows + 1);
                implicitHeight = rowsHeight
            } else {
                //restore default values if none of rows is loaded (which shows that forecast model is empty)
                neededRowSpacing = 0;
                implicitHeight = 0;

            }
            //the same for columns as for rows
            if(isColumnLoaded(leftColumn)) {
                var columnsWidth = implicitColumnWidth(leftColumn) * columns;
                neededColumnSpacing =  (parent.width - verticalHeader.width - columnsWidth) / (columns + 1)
                implicitWidth = columnsWidth
            } else {
                neededColumnSpacing = 0;
                implicitWidth = 0;
            }
        }

        property real neededRowSpacing: 0
        property real neededColumnSpacing: 0

        anchors.topMargin: neededRowSpacing
        anchors.bottomMargin: neededRowSpacing
        anchors.leftMargin: neededColumnSpacing
        anchors.rightMargin: neededColumnSpacing

        rowSpacing: neededRowSpacing
        columnSpacing: neededColumnSpacing

        delegate: ColumnLayout {
            id: dayDelegate

            visible: !!model.conditionIcon

            spacing: Math.round(Kirigami.Units.smallSpacing / 2)

            Kirigami.Icon {
                Layout.fillWidth: true
                Layout.preferredHeight: preferredIconSize
                Layout.preferredWidth: preferredIconSize

                Layout.alignment: Qt.AlignTop

                source: model.conditionIcon

                PlasmaCore.ToolTipArea {
                    id: iconToolTip
                    anchors.fill: parent
                    mainText: {
                        if (!model.condition) {
                            return "";
                        }
                        if (!model.conditionProbability) {
                            return model.condition;
                        }
                        return i18nc("certain weather condition (probability percentage)",
                            "%1 (%2%)", model.condition, model.conditionProbability);
                    }
                }
            }

            PlasmaComponents.Label {
                // Position it closer to the weather condition icon
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                Layout.topMargin: -Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing * 2
                // Fixed value, to prevent the emoji font from setting a larger height
                Layout.preferredHeight: labelFontMetrics.height

                horizontalAlignment: Text.AlignHCenter
                text: !!model.conditionProbability ? i18nc("Probability of precipitation in percentage", "☂%1%", model.conditionProbability) : "·"
                textFormat: Text.PlainText
                visible: root.futureDays.hasProbability
            }

            PlasmaComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: !isNaN(model.highTemp) && !!root.metaData?.temperatureUnit ? Util.temperatureToDisplayString(root.displayTemperatureUnit, model.highTemp, root.metaData.temperatureUnit, true) : i18nc("Short for no data available", "-")
                textFormat: Text.PlainText
                visible: !isNaN(model.highTemp) || !futureDays.isNightPresent
                Layout.preferredHeight: labelFontMetrics.height
            }

            PlasmaComponents.Label {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: !isNaN(model.lowTemp) && !!root.metaData?.temperatureUnit ? Util.temperatureToDisplayString(root.displayTemperatureUnit, model.lowTemp, root.metaData.temperatureUnit, true) : i18nc("Short for no data available", "-")
                textFormat: Text.PlainText
                visible: !isNaN(model.lowTemp) || !futureDays.isNightPresent
                Layout.preferredHeight: labelFontMetrics.height
            }
        }
    }
}
