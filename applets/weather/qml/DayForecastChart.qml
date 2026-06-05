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

    readonly property int preferredIconSize: Kirigami.Units.iconSizes.medium

    readonly property real preferredCellWidth: root.preferredIconSize + Kirigami.Units.largeSpacing * 2
    // No extra spacing needed when day and night are shown separately; the horizontal header already provides it.
    readonly property real preferredCellHeight: root.preferredIconSize + 2 * labelFontMetrics.height + Kirigami.Units.largeSpacing * (2 + (futureDays?.isNightPresent ? 0 : 1))

    readonly property real minimalSpacing: Kirigami.Units.smallSpacing

    implicitWidth: forecast.contentWidth + verticalHeader.width
    implicitHeight: forecast.contentHeight + horizontalHeader.height

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
        syncView: forecast
        clip: true
        textRole: "monthDay"
        resizableColumns: false
        interactive: false
        // Check if a night entry exists, as the TableView delegate shows a month/weekday label when no night entry is present.
        model: !!root.futureDays && root.futureDays.daysNumber > 1 && root.futureDays.isNightPresent ? root.futureDays : null

        delegate: PlasmaComponents.Label {
            text: model.monthDay ?? model.weekDay ?? ""
            textFormat: Text.PlainText
            horizontalAlignment: Text.AlignHCenter
        }
    }

    VerticalHeaderView {
        id: verticalHeader
        anchors.top: forecast.top
        anchors.left: parent.left
        syncView: forecast
        clip: true
        textRole: "period"
        resizableRows: false
        interactive: false
        model: !!root.futureDays && root.futureDays.isNightPresent ? root.futureDays : null

        delegate: PlasmaComponents.Label {
            text: model.period ?? ""
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

        clip: false

        // calculate spacing and implicit width/height when layout changed
        onLayoutChanged: {
            //check if row loaded before calculating row height to prevent rows from being shown incorrectly
            if (isRowLoaded(topRow)) {
                var rowsHeight = implicitRowHeight(topRow) * rows;
                neededRowSpacing = Math.max((parent.height - horizontalHeader.height - rowsHeight) / (rows + 1), root.minimalSpacing);
                implicitHeight = rowsHeight;
            } else {
                //restore default values if none of rows is loaded (which shows that forecast model is empty)
                neededRowSpacing = 0;
                implicitHeight = 0;
            }
            //the same for columns as for rows
            if (isColumnLoaded(leftColumn)) {
                var columnsWidth = implicitColumnWidth(leftColumn) * columns;
                neededColumnSpacing = Math.max((parent.width - verticalHeader.width - columnsWidth) / (columns + 1), root.minimalSpacing);
                implicitWidth = columnsWidth;
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

        delegate: Rectangle {
            color: Qt.alpha(Kirigami.Theme.highlightColor, 0.08)
            implicitWidth: root.preferredCellWidth
            implicitHeight: root.preferredCellHeight
            radius: Kirigami.Units.cornerRadius

            ColumnLayout {
                id: dayDelegate

                anchors.centerIn: parent

                visible: !!model.conditionIcon

                spacing: Math.round(Kirigami.Units.smallSpacing / 2)

                PlasmaComponents.Label {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                    visible: !root.futureDays?.isNightPresent
                    Layout.preferredHeight: labelFontMetrics.height
                    text: model.monthDay ?? model.weekDay ?? ""
                }

                Kirigami.Icon {
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.preferredIconSize
                    Layout.preferredWidth: root.preferredIconSize

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
                            return i18nc("certain weather condition (probability percentage)", "%1 (%2%)", model.condition, model.conditionProbability);
                        }
                    }
                }

                PlasmaComponents.Label {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: {
                        if (!isNaN(model.highTemp) && !isNaN(model.lowTemp) && !!root.metaData?.temperatureUnit) {
                            let lowTemp = Util.temperatureToDisplayString(root.displayTemperatureUnit, model.lowTemp, root.metaData.temperatureUnit, true, true);
                            let highTemp = Util.temperatureToDisplayString(root.displayTemperatureUnit, model.highTemp, root.metaData.temperatureUnit, true, true);
                            return i18nc("High and low temperature wrapper", "%1/<font color='%2'>%3</font>", highTemp, Kirigami.Theme.disabledTextColor.toString(), lowTemp);
                        }
                        if (!isNaN(model.highTemp) && !!root.metaData?.temperatureUnit) {
                            return Util.temperatureToDisplayString(root.displayTemperatureUnit, model.highTemp, root.metaData.temperatureUnit, true, true);
                        }

                        if (!isNaN(model.lowTemp) && !!root.metaData?.temperatureUnit) {
                            return Util.temperatureToDisplayString(root.displayTemperatureUnit, model.lowTemp, root.metaData.temperatureUnit, true, true);
                        }
                        return i18nc("Short for no data available", "-");
                    }
                    textFormat: Text.RichText
                    visible: !isNaN(model.highTemp) || !isNaN(model.lowTemp)
                    Layout.preferredHeight: labelFontMetrics.height
                    font.family: Kirigami.Theme.smallFont.family
                    font.pixelSize: Kirigami.Theme.smallFont.pixelSize
                }

                PlasmaComponents.Label {
                    // Position it closer to the weather condition icon
                    Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                    // Fixed value, to prevent the emoji font from setting a larger height
                    Layout.preferredHeight: labelFontMetrics.height

                    horizontalAlignment: Text.AlignHCenter
                    text: !!model.conditionProbability ? i18nc("Probability of precipitation in percentage", "☂%1%", model.conditionProbability) : "·"
                    textFormat: Text.PlainText
                    visible: root.futureDays.hasProbability
                    color: Kirigami.Theme.disabledTextColor
                }
            }
        }
    }
}
