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

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.private.weather 1.0

Item {
    id: root

    readonly property string weatherSource: plasmoid.nativeInterface.source
    readonly property int updateInterval: plasmoid.nativeInterface.updateInterval
    readonly property int displayTemperatureUnit: plasmoid.nativeInterface.displayTemperatureUnit
    readonly property int displaySpeedUnit: plasmoid.nativeInterface.displaySpeedUnit
    readonly property int displayPressureUnit: plasmoid.nativeInterface.displayPressureUnit
    readonly property int displayVisibilityUnit: plasmoid.nativeInterface.displayVisibilityUnit

    property bool connectingToSource: false
    readonly property string currentWeatherIconName: !panelModel.location ? "weather-none-available" : panelModel.currentConditionIcon
    readonly property bool needsConfiguration: !panelModel.location && !connectingToSource

    readonly property int invalidUnit: -1 //TODO: make KUnitConversion::InvalidUnit usable here

    readonly property var panelModel: {
        var model = {};
        var data = weatherDataSource.currentData;

        var todayForecastTokens = ((data && data["Short Forecast Day 0"]) || "").split("|");
        var reportTemperatureUnit = (data && data["Temperature Unit"]) || invalidUnit;

        model["location"] = (data && data["Place"]) || "";
        model["courtesy"] = (data && data["Credit"]) || "";
        model["creditUrl"] = (data && data["Credit Url"]) || "";

        model["currentConditions"] = (data && data["Current Conditions"]) || "";

        var forecastDayCount = parseInt((data && data["Total Weather Days"]) || "");
        var forecastTitle;
        if (!isNaN(forecastDayCount) && forecastDayCount > 0) {
            forecastTitle = i18ncp("Forecast period timeframe", "1 Day", "%1 Days", forecastDayCount);
        }
        model["forecastTitle"] = forecastTitle || "";

        var conditionIconName = (data && data["Condition Icon"]) || null;
        var weatherIconName;
        if (!conditionIconName ||
            conditionIconName === "weather-none-available") {
            // try icon from current weather forecast
            if (todayForecastTokens.length === 6 && todayForecastTokens[1] !== "N/U") {
                weatherIconName = Util.existingWeatherIconName(todayForecastTokens[1]);
            } else {
                weatherIconName = "weather-none-available";
            }
        } else {
            weatherIconName = Util.existingWeatherIconName(conditionIconName);
        }
        model["currentConditionIcon"] = weatherIconName;

        var temperature = (data && data["Temperature"]) || null;
        model["currentTemperature"] = temperature !== null ? Util.temperatureToDisplayString(displayTemperatureUnit, temperature, reportTemperatureUnit) : "";

        var currentDayLowTemperature;
        var currentDayHighTemperature;
        if (todayForecastTokens.length === 6) {
            var reportLowString = todayForecastTokens[4];
            if (reportLowString && reportLowString !== "N/A") {
                currentDayLowTemperature = Util.temperatureToDisplayString(displayTemperatureUnit, reportLowString, reportTemperatureUnit, true);
            }

            var reportHighString = todayForecastTokens[3];
            if (reportHighString && reportHighString !== "N/A") {
                currentDayHighTemperature = Util.temperatureToDisplayString(displayTemperatureUnit, reportHighString, reportTemperatureUnit, true);
            }
        }
        model["currentDayLowTemperature"] = currentDayLowTemperature || "";
        model["currentDayHighTemperature"] = currentDayHighTemperature || "";

        return model;
    }

    readonly property var detailsModel: {
        var model = [];
        var data = weatherDataSource.currentData;

        var reportTemperatureUnit = (data && data["Temperature Unit"]) || invalidUnit;

        var windChill = (data && data["Windchill"]) || null;
        if (windChill) {
            // Use temperature unit to convert windchill temperature
            // we only show degrees symbol not actual temperature unit
            var windChillString = Util.temperatureToDisplayString(displayTemperatureUnit, windChill, reportTemperatureUnit, false, true);

            model.push({ "text": i18n("Windchill: %1", windChillString) });
        };

        var humidex = (data && data["Humidex"]) || null;
        if (humidex) {
            // TODO: this seems wrong, does the humidex have temperature as units?
            // Use temperature unit to convert humidex temperature
            // we only show degrees symbol not actual temperature unit
            var humidexString = Util.temperatureToDisplayString(displayTemperatureUnit, humidex, reportTemperatureUnit, false, true);

            model.push({ "text": i18n("Humidex: %1", humidexString) });
        }

        var dewpoint = (data && data["Dewpoint"]) || null;
        if (dewpoint) {
            var dewpointString = Util.temperatureToDisplayString(displayTemperatureUnit, dewpoint, reportTemperatureUnit);

            model.push({ "text": i18nc("ground temperature", "Dewpoint: %1", dewpointString) });
        }

        var pressure = (data && data["Pressure"]) || null;
        if (pressure) {
            var reportPressureUnit = data["Pressure Unit"] || invalidUnit;
            var pressureText = Util.valueToDisplayString(displayPressureUnit, pressure, reportPressureUnit, 2);
            pressureText = i18n("Pressure: %1", pressureText);

            model.push({ "text": pressureText });
        }

        var pressureTendency = (data && data["Pressure Tendency"]) || null;
        if (pressureTendency) {
            var i18nPressureTendency = i18nc("pressure tendency", pressureTendency);
            i18nPressureTendency = i18nc("pressure tendency, rising/falling/steady",
                                         "Pressure Tendency: %1", i18nPressureTendency);

            model.push({ "text": i18nPressureTendency });
        }

        var visibility = (data && data["Visibility"]) || null;
        if (visibility) {
            var visibilityText;
            var reportVisibilityUnit = data["Visibility Unit"] || invalidUnit;
            if (reportVisibilityUnit !== invalidUnit) {
                visibilityText = Util.valueToDisplayString(displayVisibilityUnit, visibility, reportVisibilityUnit, 1);
                visibilityText = i18n("Visibility: %1", visibilityText);
            } else {
                visibilityText = i18n("Visibility: %1", visibility);
            }

            model.push({ "text": visibilityText });
        }

        var humidity = (data && data["Humidity"]) || null;
        if (humidity) {
            var humidityString = Util.percentToDisplayString(humidity);
            model.push({ "text": i18n("Humidity: %1", humidityString) });
        }

        var reportWindSpeedUnit = (data && data["Wind Speed Unit"]) || invalidUnit;

        var windSpeed = (data && data["Wind Speed"]) || null;
        if (windSpeed !== null && windSpeed !== "") {
            // TODO: missing check for windDirection validness
            var windDirection = data["Wind Direction"] || null;

            var windSpeedText;
            var windSpeedNumeric = (typeof windSpeed !== 'number') ? parseFloat(windSpeed) : windSpeed;
            if (!isNaN(windSpeedNumeric)) {
                if (windSpeedNumeric !== 0) {
                    windSpeedText = Util.valueToDisplayString(displaySpeedUnit, windSpeedNumeric, reportWindSpeedUnit, 1);
                    var i18nWindDirection = i18nc("wind direction", windDirection);
                    windSpeedText = i18nc("wind direction, speed","%1 %2", i18nWindDirection, windSpeedText);
                } else {
                    windSpeedText = i18nc("Wind condition", "Calm");
                }
            } else {
                windSpeedText = windSpeed;
            }

            model.push({
                "icon": windDirection,
                "text": windSpeedText
            });
        }

        var windGust = (data && data["Wind Gust"]) || null;
        if (windGust) {
            var windGustString = Util.valueToDisplayString(displaySpeedUnit, windGust, reportWindSpeedUnit, 1);
            model.push({ "text": i18n("Wind Gust: %1", windGustString) });
        }

        return model;
    }

    readonly property var forecastModel: {
        var model = [];
        var data = weatherDataSource.currentData;

        var forecastDayCount = parseInt((data && data["Total Weather Days"]) || "");
        if (isNaN(forecastDayCount) || forecastDayCount <= 0) {
            return model;
        }

        var reportTemperatureUnit = (data && data["Temperature Unit"]) || invalidUnit;

        var dayItems = [];
        var conditionItems = [];
        var hiItems = [];
        var lowItems = [];

        for (var i = 0; i < forecastDayCount; ++i) {
            var forecastDayKey = "Short Forecast Day " + i;
            var forecastDayTokens = ((data && data[forecastDayKey]) || "").split("|");

            if (forecastDayTokens.length !== 6) {
                // We don't have the right number of tokens, abort trying
                break;
            }

            dayItems.push(forecastDayTokens[0]);

            // If we see N/U (Not Used) we skip the item
            var weatherIconName = forecastDayTokens[1];
            if (weatherIconName && weatherIconName !== "N/U") {
                var iconAndToolTip = Util.existingWeatherIconName(weatherIconName);

                iconAndToolTip += "|";

                var condition = forecastDayTokens[2];
                var probability = forecastDayTokens[5];
                if (probability !== "N/U" &&
                    probability !== "N/A" &&
                    !!probability) {
                    iconAndToolTip += i18nc("certain weather condition (probability percentage)",
                                            "%1 (%2 %)", condition, probability);
                } else {
                    iconAndToolTip += condition;
                }
                conditionItems.push(iconAndToolTip);
            }

            var tempHigh = forecastDayTokens[3];
            if (tempHigh !== "N/U") {
                if (tempHigh === "N/A" || !tempHigh) {
                    hiItems.push(i18nc("Short for no data available", "-"));
                } else {
                    hiItems.push(Util.temperatureToDisplayString(displayTemperatureUnit, tempHigh,
                                                                 reportTemperatureUnit, true));
                }
            }

            var tempLow = forecastDayTokens[4];
            if (tempLow !== "N/U") {
                if (tempLow === "N/A" || !tempLow) {
                    lowItems.push(i18nc("Short for no data available", "-"));
                } else {
                    lowItems.push(Util.temperatureToDisplayString(displayTemperatureUnit, tempLow,
                                                                  reportTemperatureUnit, true));
                }
            }
        }

        if (dayItems.length) {
            model.push(dayItems);
        }
        if (conditionItems.length) {
            model.push(conditionItems);
        }
        if (hiItems.length)  {
            model.push(hiItems);
        }
        if (lowItems.length) {
            model.push(lowItems);
        }

        return model;
    }

    readonly property var noticesModel: {
        var model = [];
        var data = weatherDataSource.currentData;

        var warnings = [];
        var warningsCount = parseInt((data && data["Total Warnings Issued"]) || "");
        if (isNaN(warningsCount)) {
            warningsCount = 0;
        }
        for (var i = 0; i < warningsCount; ++i) {
            warnings.push({
                "description": data["Warning Description "+i],
                "info":        data["Warning Info "+i]
            });
        }
        model.push(warnings);

        var watches = [];
        var watchesCount = parseInt((data && data["Total Watches Issued"]) || "");
        if (isNaN(watchesCount)) {
            watchesCount = 0;
        }
        for (var i = 0; i < watchesCount; ++i) {
            watches.push({
                "description": data["Watch Description "+i],
                "info":        data["Watch Info "+i]
            });
        }
        model.push(watches);

        return model;
    }

    PlasmaCore.DataSource {
        id: weatherDataSource

        readonly property var currentData: data[weatherSource]

        engine: "weather"
        connectedSources: weatherSource
        interval: updateInterval * 60 * 1000
        onConnectedSourcesChanged: {
            if (weatherSource) {
                connectingToSource = true;
                plasmoid.busy = true;
                connectionTimeoutTimer.start();
            }
        }
        onCurrentDataChanged: {
            if (currentData) {
                connectionTimeoutTimer.stop();
                connectingToSource = false;
                plasmoid.busy = false;
            }
        }
    }

    Timer {
        id: connectionTimeoutTimer

        interval: 60 * 1000 // 1 min
        repeat: false
        onTriggered: {
            connectingToSource = false;
            plasmoid.busy = false;
            // TODO: inform user
            var sourceTokens = weatherSource.split("|");
            var foo = i18n("Weather information retrieval for %1 timed out.", sourceTokens.value(2));
        }
    }

    // workaround for now to ensure "Please Configure" tooltip
    // TODO: remove when configurationRequired works
    Plasmoid.icon: needsConfiguration ? "configure" : currentWeatherIconName
    Plasmoid.toolTipMainText: needsConfiguration ? i18nc("@info:tooltip", "Please Configure") : panelModel.location
    Plasmoid.toolTipSubText: {
        if (!panelModel.location) {
            return "";
        }
        if (panelModel.currentConditions && panelModel.currentTemperature) {
            return i18nc("%1 is the weather condition, %2 is the temperature,  both come from the weather provider",
                         "%1 %2", panelModel.currentConditions, panelModel.currentTemperature);
        }
        return panelModel.currentConditions || panelModel.currentTemperature || "";
    }
    Plasmoid.associatedApplicationUrls: panelModel.creditUrl || null

    Plasmoid.compactRepresentation: CompactRepresentation {
        model: panelModel
    }

    Plasmoid.fullRepresentation: FullRepresentation {
        model: panelModel
    }

    Component.onCompleted: {
        // workaround for missing note about being in systray or similar (kde bug #388995)
        // guess from cointainer structure data and make available to config page
        plasmoid.nativeInterface.needsToBeSquare =
            (plasmoid.parent !== null &&
            ((plasmoid.parent.pluginName === 'org.kde.plasma.private.systemtray' ||
              plasmoid.parent.objectName === 'taskItemContainer')));
    }
}
