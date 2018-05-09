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
    readonly property bool needsConfiguration: !generalModel.location && !connectingToSource

    readonly property int invalidUnit: -1 //TODO: make KUnitConversion::InvalidUnit usable here

    // model providing final display strings for observation properties
    readonly property var observationModel: {
        var model = {};
        var data = weatherDataSource.currentData;

        var reportTemperatureUnit = (data && data["Temperature Unit"]) || invalidUnit;
        var reportPressureUnit =    (data && data["Pressure Unit"])    || invalidUnit;
        var reportVisibilityUnit =  (data && data["Visibility Unit"])  || invalidUnit;
        var reportWindSpeedUnit =   (data && data["Wind Speed Unit"])  || invalidUnit;

        model["conditions"] = (data && data["Current Conditions"]) || "";

        var conditionIconName = (data && data["Condition Icon"]) || null;
        model["conditionIconName"] = conditionIconName ? Util.existingWeatherIconName(conditionIconName) : "weather-none-available";

        var temperature = (data && data["Temperature"]) || null;
        model["temperature"] = temperature !== null ? Util.temperatureToDisplayString(displayTemperatureUnit, temperature, reportTemperatureUnit) : "";

        var windchill = (data && data["Windchill"]) || null;
        // Use temperature unit to convert windchill temperature
        // we only show degrees symbol not actual temperature unit
        model["windchill"] = windchill !== null ?
            Util.temperatureToDisplayString(displayTemperatureUnit, windchill, reportTemperatureUnit, false, true) :
            "";

        var humidex = (data && data["Humidex"]) || null;
        // TODO: this seems wrong, does the humidex have temperature as units?
        // Use temperature unit to convert humidex temperature
        // we only show degrees symbol not actual temperature unit
        model["humidex"] = humidex !== null ?
            Util.temperatureToDisplayString(displayTemperatureUnit, humidex, reportTemperatureUnit, false, true) :
            "";

        var dewpoint = (data && data["Dewpoint"]) || null;
        model["dewpoint"] = dewpoint !== null ?
            Util.temperatureToDisplayString(displayTemperatureUnit, dewpoint, reportTemperatureUnit) : "";

        var pressure = (data && data["Pressure"]) || null;
        model["pressure"] = pressure ?
            Util.valueToDisplayString(displayPressureUnit, pressure, reportPressureUnit, 2) : "";

        var pressureTendency = (data && data["Pressure Tendency"]) || null;
        model["pressureTendency"] = pressureTendency ? i18nc("pressure tendency", pressureTendency) : "";

        var visibility = (data && data["Visibility"]) || null;
        model["visibility"] = visibility ?
            ((reportVisibilityUnit !== invalidUnit) ?
                Util.valueToDisplayString(displayVisibilityUnit, visibility, reportVisibilityUnit, 1) : visibility) :
            "";

        var humidity = (data && data["Humidity"]) || null;
        model["humidity"] = humidity ? Util.percentToDisplayString(humidity) : "";

        // TODO: missing check for windDirection validness
        var windDirection = (data && data["Wind Direction"]) || null;
        var windSpeed = (data && data["Wind Speed"]) || null;
        var windSpeedText;
        if (windSpeed !== null && windSpeed !== "") {
            var windSpeedNumeric = (typeof windSpeed !== 'number') ? parseFloat(windSpeed) : windSpeed;
            if (!isNaN(windSpeedNumeric)) {
                if (windSpeedNumeric !== 0) {
                    windSpeedText = Util.valueToDisplayString(displaySpeedUnit, windSpeedNumeric, reportWindSpeedUnit, 1);
                } else {
                    windSpeedText = i18nc("Wind condition", "Calm");
                }
            } else {
                // TODO: i18n?
                windSpeedText = windSpeed;
            }
        }
        model["windSpeed"] = windSpeedText || "";
        model["windDirectionId"] = windDirection || "";
        model["windDirection"] = windDirection ? i18nc("wind direction", windDirection) : "";

        var windGust = (data && data["Wind Gust"]) || null;
        model["windGust"] = windGust ? Util.valueToDisplayString(displaySpeedUnit, windGust, reportWindSpeedUnit, 1) : "";

        return model;
    }

    readonly property var generalModel: {
        var model = {};
        var data = weatherDataSource.currentData;

        var todayForecastTokens = ((data && data["Short Forecast Day 0"]) || "").split("|");

        model["location"] = (data && data["Place"]) || "";
        model["courtesy"] = (data && data["Credit"]) || "";
        model["creditUrl"] = (data && data["Credit Url"]) || "";

        var forecastDayCount = parseInt((data && data["Total Weather Days"]) || "");
        var forecastTitle;
        if (!isNaN(forecastDayCount) && forecastDayCount > 0) {
            forecastTitle = i18ncp("Forecast period timeframe", "1 Day", "%1 Days", forecastDayCount);
        }
        model["forecastTitle"] = forecastTitle || "";

        var conditionIconName = observationModel.conditionIconName;
        if (!conditionIconName ||
            conditionIconName === "weather-none-available") {

            // try icon from current weather forecast
            if (todayForecastTokens.length === 6 && todayForecastTokens[1] !== "N/U") {
                conditionIconName = Util.existingWeatherIconName(todayForecastTokens[1]);
            } else {
                conditionIconName = "weather-none-available";
            }
        }
        model["currentConditionIconName"] = conditionIconName;

        return model;
    }

    readonly property var detailsModel: {
        var model = [];

        if (observationModel.windchill) {
            model.push({ "text": i18n("Windchill: %1", observationModel.windchill) });
        };

        if (observationModel.humidex) {
            model.push({ "text": i18n("Humidex: %1", observationModel.humidex) });
        }

        if (observationModel.dewpoint) {
            model.push({ "text": i18nc("ground temperature", "Dewpoint: %1", observationModel.dewpoint) });
        }

        if (observationModel.pressure) {
            model.push({ "text": i18n("Pressure: %1", observationModel.pressure) });
        }

        if (observationModel.pressureTendency) {
            model.push({ "text": i18nc("pressure tendency, rising/falling/steady",
                                       "Pressure Tendency: %1", observationModel.pressureTendency) });
        }

        if (observationModel.visibility) {
            model.push({ "text": i18n("Visibility: %1", observationModel.visibility) });
        }

        if (observationModel.humidity) {
            model.push({ "text": i18n("Humidity: %1", observationModel.humidity) });
        }

        if (observationModel.windGust) {
            model.push({ "text": i18n("Wind Gust: %1",observationModel. windGust) });
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

    // workaround for now to ensure "Please configure" tooltip
    // TODO: remove when configurationRequired works
    Plasmoid.icon: needsConfiguration ? "configure" : generalModel.currentConditionIconName
    Plasmoid.toolTipMainText: needsConfiguration ? i18nc("@info:tooltip", "Please configure") : generalModel.location
    Plasmoid.toolTipSubText: {
        if (!generalModel.location) {
            return "";
        }
        var tooltips = [];
        var temperature = plasmoid.nativeInterface.temperatureShownInTooltip ? observationModel.temperature : null;
        if (observationModel.conditions && temperature) {
            tooltips.push(i18nc("weather condition + temperature",
                                "%1 %2", observationModel.conditions, temperature));
        } else if (observationModel.conditions || temperature) {
            tooltips.push(observationModel.conditions || temperature);
        }
        if (plasmoid.nativeInterface.windShownInTooltip) {
            if (observationModel.windGust) {
                tooltips.push(i18nc("winddirection windspeed (windgust)", "%1 %2 (%3)",
                                    observationModel.windDirection, observationModel.windSpeed, observationModel.windGust));
            } else {
                tooltips.push(i18nc("winddirection windspeed", "%1 %2",
                                    observationModel.windDirection, observationModel.windSpeed));
            }
        }
        if (plasmoid.nativeInterface.pressureShownInTooltip && observationModel.pressure) {
            if (observationModel.pressureTendency) {
                tooltips.push(i18nc("pressure (tendency)", "%1 (%2)",
                                    observationModel.pressure, observationModel.pressureTendency));
            } else {
                tooltips.push(observationModel.pressure);
            }
        }
        if (plasmoid.nativeInterface.humidityShownInTooltip && observationModel.humidity) {
            tooltips.push(i18n("Humidity: %1", observationModel.humidity));
        }

        return tooltips.join("\n");
    }
    Plasmoid.associatedApplicationUrls: generalModel.creditUrl || null

    Plasmoid.compactRepresentation: CompactRepresentation {
        generalModel: root.generalModel
        observationModel: root.observationModel
    }

    Plasmoid.fullRepresentation: FullRepresentation {
        generalModel: root.generalModel
        observationModel: root.observationModel
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
