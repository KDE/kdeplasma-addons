/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.core as PlasmaCore

GridLayout {
    id: root

    property var metaData: null
    property var lastObservation: null
    property var station: null
    property var futureDays: null

    property int displayWindSpeedUnit: 0
    property int displayTemperatureUnit: 0

    readonly property bool isTemperaturePresent: !!root.lastObservation?.temperature && !!root.metaData?.temperatureUnit

    function feelsLikeTemperature(windchill, heatIndex, humidex) {
        if (!root.isTemperaturePresent) {
            return "";
        }

        let feelsTemperature;
        if(windchill) {
            feelsTemperature = windchill;
        } else if (heatIndex) {
            feelsTemperature = heatIndex;
        } else if (humidex) {
            feelsTemperature = humidex;
        }

        if (!feelsTemperature) {
            return null;
        }

        return feelsTemperature;
    }

    readonly property int sideWidth: Math.max(
        windSpeedLabel.implicitWidth,
        tempLabel.implicitWidth,
        windSpeedDirection.naturalSize.width
    )

    Layout.minimumWidth: Math.max(
        Math.min(locationLabel.implicitWidth, Kirigami.Units.gridUnit * 25),
        (sideWidth + columnSpacing) * 2 + Kirigami.Units.iconSizes.huge /* conditionIcon.Layout.minimumWidth */
    )

    columnSpacing: Kirigami.Units.largeSpacing
    rowSpacing: Kirigami.Units.largeSpacing

    columns: 3

    Kirigami.Heading {
        id: locationLabel

        Layout.row: 0
        Layout.column: 0
        Layout.columnSpan: 3
        Layout.fillWidth: true

        elide: Text.ElideRight

        visible: !!root.station?.place

        text: visible ? root.station.place : ""
        textFormat: Text.PlainText

        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            mainText: locationLabel.visible ? root.station.place : ""
            visible: locationLabel.truncated
        }
    }

    ColumnLayout {
        Layout.row: 1
        Layout.column: 0
        Layout.fillWidth: true
        Layout.preferredWidth: 25 // 25% of the view
        Layout.minimumWidth: sideWidth

        spacing: Kirigami.Units.smallSpacing

        PlasmaComponents.Label {
            id: tempLabel
            Layout.fillWidth: true

            visible: root.isTemperaturePresent

            font.pixelSize: Kirigami.Units.iconSizes.medium
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.NoWrap
            textFormat: Text.PlainText

            text: root.isTemperaturePresent ? Util.temperatureToDisplayString(root.displayTemperatureUnit, root.lastObservation.temperature, root.metaData.temperatureUnit, true, false) : ""
        }

        PlasmaComponents.Label {
            id: feelsLikeLabel
            Layout.fillWidth: true

            readonly property bool isFeelsLikeTemperaturePresent: isTemperaturePresent  && (!!root.lastObservation.heatIndex  || !!root.lastObservation.windchill || !!root.lastObservation.humidex)

            visible: {
                if (feelsLikeLabel.isFeelsLikeTemperaturePresent) {
                    let feelsTemperature = feelsLikeTemperature(root.lastObservation.windchill, root.lastObservation.heatIndex, root.lastObservation.humidex);
                    return feelsTemperature !== "" && feelsTemperature !== root.lastObservation.temperature
                }

                return false;
            }

            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            textFormat: Text.PlainText

            text: {
                if (feelsLikeLabel.isFeelsLikeTemperaturePresent) {
                    let feelsTemperature = feelsLikeTemperature(root.lastObservation.windchill, root.lastObservation.heatIndex, root.lastObservation.humidex);
                    let feelsTemperatureString = Util.temperatureToDisplayString(root.displayTemperatureUnit, feelsTemperature, root.metaData.temperatureUnit, true, false);
                    return i18nc("@label %1 is the perceived temperature due to conditions like wind or humidity. Use the common phrasing for this concept and keep it short, adding a colon if necessary",
                        "Feels like %1", feelsTemperatureString);
                }
                return "";
            }
        }
    }

    Kirigami.Icon {
        id: conditionIcon

        visible: !!root.lastObservation?.conditionIcon || !!root.futureDays?.firstDayIcon

        Layout.row: 1
        Layout.column: 1
        Layout.minimumHeight: Kirigami.Units.iconSizes.huge
        Layout.minimumWidth: Kirigami.Units.iconSizes.huge
        Layout.preferredHeight: Layout.minimumHeight
        Layout.fillWidth: true
        // All the items have `fillWidth: true`, so the layout weights each
        // contribution and splits the space accordingly to their proportion.
        Layout.preferredWidth: 50 // 50% of the view

        source: {
            //check if there is the icon from last observation and if it exists return it
            if (!!root.lastObservation?.conditionIcon && root.lastObservation.conditionIcon !== "weather-none-available") {
                return root.lastObservation.conditionIcon;
            }
            //if the icon from last observation not exists use first icon from forecast
            if (!!root.futureDays?.firstDayIcon) {
                return root.futureDays.firstDayIcon;
            }
            //if there are no icons then use default unavailable icon
            return "weather-none-available";
        }
    }

    PlasmaComponents.Label {
        id: conditionLabel

        visible: !!root.lastObservation?.currentConditions

        Layout.row: 2
        Layout.column: 0
        Layout.columnSpan: 3
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

        text: visible ? root.lastObservation.currentConditions : ""
        textFormat: Text.PlainText
    }

    Item {
        Layout.row: 1
        Layout.column: 2
        Layout.fillWidth: true
        Layout.preferredWidth: 25 // 25% of the view
        Layout.minimumWidth: sideWidth
        Layout.alignment: Qt.AlignCenter

        implicitHeight: windSpeedDirection.implicitHeight + windSpeedLabel.implicitHeight

        KSvg.SvgItem {
            id: windSpeedDirection

            anchors.horizontalCenter: parent.horizontalCenter
            implicitWidth: Kirigami.Units.iconSizes.medium
            implicitHeight: Kirigami.Units.iconSizes.medium

            imagePath: "weather/wind-arrows"
            elementId: visible ? root.lastObservation.windDirection : ""

            visible: !!root.lastObservation?.windDirection
        }

        PlasmaComponents.Label {
            id: windSpeedLabel

            visible: true

            readonly property bool isWindSpeedPresent: !!root.lastObservation?.windSpeed && !!root.metaData?.windSpeedUnit && root.lastObservation.windSpeed !== 0.0

            anchors {
                top: windSpeedDirection.bottom
                horizontalCenter: parent.horizontalCenter
            }

            text: isWindSpeedPresent ? Util.valueToDisplayString(root.displayWindSpeedUnit, root.lastObservation.windSpeed, root.metaData.windSpeedUnit, 1) : i18nc("Wind condition", "Calm");
            textFormat: Text.PlainText
        }
    }

}
