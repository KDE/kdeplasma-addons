/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

ColumnLayout {
    id: root

    Layout.fillWidth: true

    property var futureHours: null
    property var metaData: null

    property int invalidUnit: 0
    property int displayTemperatureUnit: 0

    readonly property int minimalSpacing: Kirigami.Units.smallSpacing * 2

    property int currentIndex: 0

    property int hoursPerPage: 0

    spacing: 0

    HourlyForecastList {
        id: forecastList

        snapMode: ListView.SnapToItem

        Layout.fillWidth: true

        spacing: {
            if (!currentItem || root.hoursPerPage == 0) {
                return 0;
            }
            const displayedItemCount = Math.min(count, root.hoursPerPage);
            const itemWidth = currentItem.implicitWidth;
            return displayedItemCount > 1 ? (width - displayedItemCount * itemWidth) / (displayedItemCount - 1) : 0;
        }

        metaData: root.metaData
        forecastModel: root.futureHours

        invalidUnit: root.invalidUnit
        displayTemperatureUnit: root.displayTemperatureUnit

        onWidthChanged: Qt.callLater(updatePageSize)
        Component.onCompleted: Qt.callLater(updatePageSize)

        // Calculate page size dynamically to fit as many forecasts as possible
        function updatePageSize() {
            if (!currentItem) {
                root.hoursPerPage = 0;
                return;
            }
            root.hoursPerPage = Math.max(1, Math.floor((width + root.minimalSpacing) / (currentItem.implicitWidth + root.minimalSpacing)));
        }
    }

    ForecastNavigation {
        Layout.alignment: Qt.AlignHCenter

        visible: forecastList.count > root.hoursPerPage

        currentIndex: root.currentIndex
        pageSize: root.hoursPerPage
        count: forecastList.count
        maxWidth: root.width

        onGoToIndex: index => {
            root.currentIndex = index;
            forecastList.positionViewAtIndex(index, ListView.Beginning);
        }
    }
}
