/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QtGraphs as Graphs

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.weatherdata as WeatherData

ColumnLayout {
    id: root

    property var futureHours: null
    property var futureHoursPoints: null
    property var metaData: null

    property int invalidUnit: 0
    property int displayTemperatureUnit: 0

    readonly property real minimalSpacing: Kirigami.Units.smallSpacing

    readonly property real preferredGraphHeight: Kirigami.Units.iconSizes.enormous

    property int currentIndex: 0

    readonly property int pageSize: Math.max(1, Math.floor((width + minimalSpacing) / (forecastInfo.itemWidth + minimalSpacing)))
    readonly property real pageWidth: pageSize * (forecastInfo.itemWidth + forecastInfo.spacing)
    readonly property real availableContentWidth: width - 2 * Kirigami.Units.iconSizes.small

    // On layout resize, re-evaluate the current scroll position.
    // This ensures the view stays on the same logical page and prevents
    // contentX from exceeding the last valid scroll position after pageWidth changes.
    onPageWidthChanged: {
        root.scrollToIndex(root.currentIndex);
    }

    function scrollToIndex(index) {
        currentIndex = index;
        // Compute the horizontal offset for the requested page.
        // Each "page" is one forecast item plus spacing.
        const xPosition = index * (forecastInfo.itemWidth + forecastInfo.spacing);
        const xLastPage = forecastFlickable.contentWidth - pageWidth;
        //last page: clamp to avoid empty space at the end
        forecastFlickable.contentX = xPosition < xLastPage ? xPosition : xLastPage;
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        PlasmaComponents.ToolButton {
            Layout.fillHeight: true
            icon.name: "go-previous"
            enabled: root.currentIndex > 0
            onClicked: {
                const newIndex = Math.max(0, root.currentIndex - root.pageSize);
                root.scrollToIndex(newIndex);
            }
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
        }

        Flickable {
            id: forecastFlickable
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitWidth: forecastInfo.itemWidth * 7
            implicitHeight: contentHeight
            contentWidth: forecastInfo.implicitWidth
            contentHeight: forecastView.implicitHeight
            clip: true
            interactive: false

            ColumnLayout {
                id: forecastView
                anchors.fill: parent
                ForecastGraph {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.preferredGraphHeight
                    Layout.topMargin: root.minimalSpacing
                    Layout.bottomMargin: root.minimalSpacing

                    marginLeft: forecastInfo.itemWidth / 2
                    marginRight: marginLeft

                    pointsModel: root.futureHoursPoints

                    xSection: WeatherData.FutureHoursPoints.Timestamp
                    ySection: WeatherData.FutureHoursPoints.Temperature

                    toolTipTextFunction: function (pointIndex) {
                        if (!root.futureHours) {
                            return "";
                        }
                        let index = root.futureHours.index(pointIndex, 0);
                        return root.futureHours.data(index, WeatherData.FutureHours.Condition);
                    }
                }

                HourlyForecastList {
                    id: forecastInfo

                    readonly property real itemWidth: forecastInfo.currentItem?.implicitWidth

                    // If the page has fewer items than the available pageSize, distribute the
                    // remaining horizontal space across spacing so items stay evenly spread
                    // and visually balanced.
                    readonly property int displayedItemCount: Math.min(count, root.pageSize)

                    implicitWidth: itemWidth * count + (spacing * count - 1)

                    showConditionIcon: false
                    showBackground: false

                    Layout.fillWidth: true

                    // Dynamic spacing ensures items evenly fill the available page width,
                    // adapting automatically to resize events and varying page sizes.
                    spacing: displayedItemCount > 1 ? (forecastFlickable.width - displayedItemCount * itemWidth) / (displayedItemCount - 1) : 0

                    metaData: root.metaData
                    forecastModel: root.futureHours

                    invalidUnit: root.invalidUnit
                    displayTemperatureUnit: root.displayTemperatureUnit
                }
            }
        }
        PlasmaComponents.ToolButton {
            Layout.fillHeight: true
            icon.name: "go-next"
            enabled: root.currentIndex + root.pageSize < forecastInfo.count
            onClicked: {
                const newIndex = Math.min(forecastInfo.count - 1, root.currentIndex + root.pageSize);
                root.scrollToIndex(newIndex);
            }
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
        }
    }

    PlasmaComponents.PageIndicator {
        id: indicator
        count: Math.ceil(forecastInfo.count / root.pageSize)

        currentIndex: Math.floor(root.currentIndex / root.pageSize)

        Layout.alignment: Qt.AlignHCenter

        visible: count > 1

        delegate: Rectangle {
            width: Math.round(Kirigami.Units.gridUnit * 0.5)
            height: width
            radius: width / 2

            color: index === indicator.currentIndex ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
            opacity: index === indicator.currentIndex ? 1.0 : 0.4

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    root.scrollToIndex(index * root.pageSize);
                }
            }

            Behavior on opacity {
                OpacityAnimator {
                    duration: Kirigami.Units.shortDuration
                }
            }
        }
    }
}
