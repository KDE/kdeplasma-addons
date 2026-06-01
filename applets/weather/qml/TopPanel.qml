/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.core as PlasmaCore
import plasma.applet.org.kde.plasma.weather

GridLayout {
    id: root

    property var metaData: null
    property var lastObservation: null
    property var station: null
    property var warnings: null
    property var futureDays: null

    property int displayWindSpeedUnit: 0
    property int displayTemperatureUnit: 0

    readonly property bool isTemperaturePresent: !!root.lastObservation?.temperature && !!root.metaData?.temperatureUnit

    signal openWarnings

    function feelsLikeTemperature(windchill, heatIndex, humidex) {
        if (!root.isTemperaturePresent) {
            return "";
        }

        let feelsTemperature;
        if (windchill) {
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

    readonly property int sideWidth: Math.max(tempLabel.implicitWidth)

    Layout.minimumWidth: Math.max(Math.min(locationLabel.implicitWidth, Kirigami.Units.gridUnit * 25), (sideWidth + columnSpacing) * 2 + Kirigami.Units.iconSizes.huge /* conditionIcon.Layout.minimumWidth */
    )

    columnSpacing: Kirigami.Units.largeSpacing
    rowSpacing: Kirigami.Units.largeSpacing

    columns: 3

    RowLayout {
        Layout.fillWidth: true
        Layout.columnSpan: 3

        Kirigami.Heading {
            id: locationLabel

            Layout.fillWidth: true

            elide: Text.ElideRight

            visible: !!root.station?.place

            text: visible ? root.station.place : ""
            textFormat: Text.PlainText

            PlasmaCore.ToolTipArea {
                mainText: locationLabel.visible ? root.station.place : ""
                visible: locationLabel.truncated
            }
        }

        QQC2.Button {
            Layout.alignment: Qt.AlignRight
            text: !!root.warnings ? i18ncp("@title:tab %1 is the number of weather notices (alerts, warnings, watches, ...) issued", "%1 Notice", "%1 Notices", root.warnings.count) : ""
            icon.name: !!root.warnings && root.warnings.maxPriorityCount >= 2 ? 'data-warning-symbolic' : 'data-information-symbolic'
            visible: !!root.warnings && root.warnings.count !== 0
            flat: true

            onClicked: root.openWarnings()
        }
    }

    Kirigami.Icon {
        id: conditionIcon

        visible: !!root.lastObservation?.conditionIcon || !!root.futureDays?.firstDayIcon

        Layout.minimumHeight: Kirigami.Units.iconSizes.huge
        Layout.minimumWidth: Kirigami.Units.iconSizes.huge
        Layout.preferredHeight: Layout.minimumHeight
        // All the items have `fillWidth: true`, so the layout weights each
        // contribution and splits the space accordingly to their proportion.
        Layout.preferredWidth: 50 // 50% of the view

        fallback: Util.unknownWeatherIcon
        source: {
            //check if there is the icon from last observation and if it exists return it
            if (!!root.lastObservation?.conditionIcon && root.lastObservation.conditionIcon !== Util.unknownWeatherIcon) {
                return root.lastObservation.conditionIcon;
            }
            //if the icon from last observation not exists use first icon from forecast
            if (!!root.futureDays?.firstDayIcon) {
                return root.futureDays.firstDayIcon;
            }
            //if there are no icons then use default unavailable icon
            return Util.unknownWeatherIcon;
        }
    }

    ColumnLayout {
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
            wrapMode: Text.NoWrap
            textFormat: Text.PlainText

            text: root.isTemperaturePresent ? Util.temperatureToDisplayString(root.displayTemperatureUnit, root.lastObservation.temperature, root.metaData.temperatureUnit, true, false) : ""
        }

        PlasmaComponents.Label {
            id: feelsLikeLabel
            Layout.fillWidth: true

            readonly property bool isFeelsLikeTemperaturePresent: isTemperaturePresent && (!!root.lastObservation.heatIndex || !!root.lastObservation.windchill || !!root.lastObservation.humidex)

            visible: {
                if (feelsLikeLabel.isFeelsLikeTemperaturePresent) {
                    let feelsTemperature = feelsLikeTemperature(root.lastObservation.windchill, root.lastObservation.heatIndex, root.lastObservation.humidex);
                    return feelsTemperature !== "" && feelsTemperature !== root.lastObservation.temperature;
                }

                return false;
            }

            wrapMode: Text.Wrap
            textFormat: Text.PlainText

            text: {
                if (feelsLikeLabel.isFeelsLikeTemperaturePresent) {
                    let feelsTemperature = feelsLikeTemperature(root.lastObservation.windchill, root.lastObservation.heatIndex, root.lastObservation.humidex);
                    let feelsTemperatureString = Util.temperatureToDisplayString(root.displayTemperatureUnit, feelsTemperature, root.metaData.temperatureUnit, true, false);
                    return i18nc("@label %1 is the perceived temperature due to conditions like wind or humidity. Use the common phrasing for this concept and keep it short, adding a colon if necessary", "Feels like %1", feelsTemperatureString);
                }
                return "";
            }
        }

        PlasmaComponents.Label {
            id: conditionLabel

            visible: !!root.lastObservation?.currentConditions

            Layout.fillWidth: true

            text: visible ? root.lastObservation.currentConditions : ""

            wrapMode: Text.Wrap
            textFormat: Text.PlainText
        }
    }
}
