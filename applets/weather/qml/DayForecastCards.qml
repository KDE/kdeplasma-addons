/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

Item {
    id: root

    property var futureDays: null
    property var metaData: null

    property int invalidUnit: 0
    property int displayTemperatureUnit: 0

    property bool showConditionIcon: true
    property bool showBackground: true

    readonly property alias rowHeight: forecast.rowHeight
    readonly property alias columnWidth: forecast.columnWidth
    readonly property alias rowSpacing: forecast.rowSpacing
    readonly property alias columnSpacing: forecast.columnSpacing
    readonly property alias horizontalHeaderHeight: horizontalHeader.implicitHeight
    readonly property alias verticalHeaderWidth: verticalHeader.implicitWidth

    readonly property real minimalSpacing: Kirigami.Units.smallSpacing

    //Limit the number of delegates to 7 to prevent broken layout at the minimal applet size
    readonly property int daysCount: Math.min(forecast.columns, 7)

    implicitWidth: forecast.implicitWidth + minimalSpacing * 2 + verticalHeader.width
    implicitHeight: forecast.implicitHeight + minimalSpacing * 2 + horizontalHeader.height

    HorizontalHeaderView {
        id: horizontalHeader
        anchors.left: forecast.left
        anchors.top: parent.top
        syncView: forecast
        textRole: "timestamp"
        resizableColumns: false
        interactive: false
        // Check if a night entry exists, as the TableView delegate shows a month/weekday label when no night entry is present.
        model: (!!root.futureDays && root.futureDays.daysNumber > 1 && root.futureDays.isNightPresent) ? root.futureDays : null

        delegate: PlasmaComponents.Label {
            text: model.timestamp.toLocaleString(Qt.locale(), "ddd") ?? ""
            textFormat: Text.PlainText
            horizontalAlignment: Text.AlignHCenter
        }
    }

    VerticalHeaderView {
        id: verticalHeader
        anchors.top: forecast.top
        anchors.left: parent.left
        syncView: forecast
        textRole: "period"
        resizableRows: false
        interactive: false
        model: (!!root.futureDays && root.futureDays.isNightPresent) ? root.futureDays : null

        delegate: PlasmaComponents.Label {
            text: model.period ?? ""
            textFormat: Text.PlainText
        }
    }

    TableView {
        id: forecast

        property real columnWidth: implicitColumnWidth(leftColumn)
        property real rowHeight: implicitRowHeight(topRow)

        interactive: false

        anchors.left: verticalHeader.right
        anchors.top: horizontalHeader.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        model: root.futureDays

        // calculate spacing to fill the view when layout changed and set the implicitHeight/Width
        onLayoutChanged: {
            //check if row loaded before calculating row height to prevent rows from being shown incorrectly
            if (isRowLoaded(topRow)) {
                rowHeight = implicitRowHeight(topRow);
                const rowsHeight = rowHeight * rows;
                neededRowSpacing = Math.max((parent.height - horizontalHeader.height - rowsHeight) / (rows + 1), root.minimalSpacing);
                implicitHeight = rowsHeight + (rows - 1) * root.minimalSpacing;
            } else {
                //restore default spacing if none of rows is loaded (which shows that forecast model is empty)
                neededRowSpacing = 0;
                implicitWidth = 0;
            }
            //the same for columns as for rows
            if (isColumnLoaded(leftColumn)) {
                columnWidth = implicitColumnWidth(leftColumn);
                const columnsWidth = columnWidth * root.daysCount;
                neededColumnSpacing = Math.max((parent.width - verticalHeader.width - columnsWidth) / (root.daysCount + 1), root.minimalSpacing);
                implicitWidth = columnsWidth + (root.daysCount - 1) * root.minimalSpacing;
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

        delegate: ForecastDelegate {
            showBackground: root.showBackground
            showConditionIcon: root.showConditionIcon
            showTimeHeader: !root.futureDays?.isNightPresent
            hasProbability: !!root.futureDays?.hasProbability
            temperatureUnit: root.metaData?.temperatureUnit || root.invalidUnit
            displayTemperatureUnit: root.displayTemperatureUnit
            timeFormat: "ddd"
        }
    }
}
