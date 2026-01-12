/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore

PlasmoidItem {
    id: root

    ForecastControl {
        id: forecastControl
        Component.onCompleted: weatherPlaceInfoChanged()

        //If forecast contains new placeInfo then save it
        onForecastChanged: {
            const station = forecastControl.forecast?.station;
            if (!station?.newPlaceInfo) {
                return;
            }
            //Reset the old source property if present as it is not needed anymore
            Plasmoid.configuration.source = "";
            Plasmoid.configuration.placeInfo = station.newPlaceInfo;
            Plasmoid.configuration.placeDisplayName = station.place;
        }
    }

    onWeatherPlaceInfoChanged: {
        if (weatherPlaceInfo.length > 0) {
            forecastControl.setUpdateInterval(root.updateInterval);
            forecastControl.setForecastLocation(root.weatherProvider, root.weatherPlaceInfo);
        //If the old source property present try to fetch forecast for it
        } else if (Plasmoid.configuration.source.length > 0) {
            const sourceDetails = Plasmoid.configuration.source.split('|');
            forecastControl.setUpdateInterval(root.updateInterval);
            Plasmoid.configuration.provider = sourceDetails[0];
            forecastControl.setForecastLocation(sourceDetails[0], Plasmoid.configuration.source);
        }
    }

    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground

    readonly property bool inPanel: [
        PlasmaCore.Types.TopEdge,
        PlasmaCore.Types.RightEdge,
        PlasmaCore.Types.BottomEdge,
        PlasmaCore.Types.LeftEdge,
    ].includes(Plasmoid.location)

    readonly property string weatherProvider: Plasmoid.configuration.provider
    readonly property string weatherPlaceInfo: Plasmoid.configuration.placeInfo
    readonly property int updateInterval: Plasmoid.configuration.updateInterval
    readonly property int displayTemperatureUnit: Plasmoid.configuration.temperatureUnit || TemperatureUnitListModel.defaultUnit
    readonly property int displaySpeedUnit: Plasmoid.configuration.speedUnit || WindSpeedUnitListModel.defaultUnit
    readonly property int displayPressureUnit: Plasmoid.configuration.pressureUnit ||  PressureUnitListModel.defaultUnit
    readonly property int displayVisibilityUnit: Plasmoid.configuration.visibilityUnit || VisibilityUnitListModel.defaultUnit

    property alias status: forecastControl.status

    readonly property int invalidUnit: -1 //TODO: make KUnitConversion::InvalidUnit usable here

    function symbolicizeIconName(iconName) {
        const symbolicSuffix = "-symbolic";
        if (iconName.endsWith(symbolicSuffix)) {
            return iconName;
        }

        return iconName + symbolicSuffix;
    }

    function windSpeedToString(windSpeed, windSpeedUnit) {
        let windSpeedText;
        if (windSpeed !== 0) {
            windSpeedText = Util.valueToDisplayString(displaySpeedUnit, windSpeed, windSpeedUnit, 1);
        } else {
            windSpeedText = i18nc("Wind condition", "Calm");
        }

        return windSpeedText;
    }

    Plasmoid.icon: {
        let iconName;
        if (status === ForecastControl.NeedsConfiguration) {
            iconName = "weather-clouds-symbolic";
        } else if (!!forecastControl.forecast?.lastObservation?.conditionIcon) {
            iconName = forecastControl.forecast.lastObservation.conditionIcon;
        } else if (!!forecastControl.forecast?.futureDays && forecastControl.forecast.futureDays.daysNumber > 0) {
            iconName = Util.existingWeatherIconName(forecastControl.forecast?.futureDays.firstDayIcon);
        } else {
            iconName = "weather-none-available"
        }

        if (inPanel) {
            iconName = symbolicizeIconName(iconName);
        }

        return iconName;
    }
    Plasmoid.title: status === ForecastControl.NeedsConfiguration ? i18nc("@info:usagetip Minimize the length of this string so it is as short as possible! Omitting or changing words is fine as long as the original meaning is retained.", "Set up Weather Report…") : null
    Plasmoid.busy: status === ForecastControl.Connecting
    Plasmoid.configurationRequired: status === ForecastControl.NeedsConfiguration
    Plasmoid.status: status === ForecastControl.NeedsConfiguration ? PlasmaCore.Types.PassiveStatus : PlasmaCore.Types.ActiveStatus

    toolTipMainText: !!forecastControl.forecast?.station?.place || status === !ForecastControl.NeedsConfiguration ? forecastControl.forecast.station.place : ""

    toolTipSubText: {
        if (status === ForecastControl.NeedsConfiguration) {
            return i18nc("@info:tooltip", "Click to choose a location and monitor the weather there")
        }

        if (!forecastControl.forecast?.station?.place) {
            return "";
        }

        const tooltips = [];

        let isTemperaturePresent = !!forecastControl.forecast.lastObservation?.temperature && !!forecastControl.forecast.metaData?.temperatureUnit;
        if (Plasmoid.configuration.showTemperatureInTooltip && isTemperaturePresent) {
            let temperature = Util.temperatureToDisplayString(root.displayTemperatureUnit, forecastControl.forecast.lastObservation.temperature, forecastControl.forecast.metaData.temperatureUnit, true, false)
            if (!!forecastControl.forecast.lastObservation?.currentConditions && temperature) {
                tooltips.push(i18nc("weather condition + temperature",
                    "%1 %2", forecastControl.forecast.lastObservation.currentConditions, temperature));
            } else if (!!forecastControl.forecast.lastObservation?.currentConditions || temperature) {
                tooltips.push(forecastControl.forecast.lastObservation.currentConditions || temperature);
            }
        }

        let isWindSpeedPresent = !!forecastControl.forecast.lastObservation?.windSpeed && !!forecastControl.forecast.metaData?.windSpeedUnit;
        if (Plasmoid.configuration.showWindInTooltip && isWindSpeedPresent) {
            let windSpeed = windSpeedToString(forecastControl.forecast.lastObservation.windSpeed, forecastControl.forecast.metaData.windSpeedUnit);
            if (!!forecastControl.forecast.lastObservation?.windDirection && forecastControl.forecast.lastObservation.windDirection !== "") {
                let windDirection =  i18nc("wind direction", forecastControl.forecast.lastObservation.windDirection);
                if (!!forecastControl.forecast.lastObservation.windGust && forecastControl.forecast.lastObservation.windGust !== 0.0) {
                    let windGust = Util.valueToDisplayString(root.displaySpeedUnit, forecastControl.forecast.lastObservation.windGust, forecastControl.forecast.metaData.windSpeedUnit, 1);
                    tooltips.push(i18nc("winddirection windspeed (windgust)", "%1 %2 (%3)",
                        windDirection, windSpeed, windGust));
                } else {
                    tooltips.push(i18nc("winddirection windspeed", "%1 %2",
                        windDirection, windSpeed));
                }
            } else {
                tooltips.push(windSpeed);
            }
        }

        let isPressurePresent = !!forecastControl.forecast.lastObservation?.pressure && !!forecastControl.forecast.metaData?.pressureUnit;
        if (Plasmoid.configuration.showPressureInTooltip && isPressurePresent) {
            const pressure = Util.valueToDisplayString(root.displayPressureUnit, forecastControl.forecast.lastObservation.pressure, forecastControl.forecast.metaData.pressureUnit, 2);
            if (!!forecastControl.forecast.lastObservation?.pressureTendency) {
                tooltips.push(i18nc("pressure (tendency)", "%1 (%2)", pressure, forecastControl.forecast.lastObservation.pressureTendency));
            } else {
                tooltips.push(pressure);
            }
        }

        let isHumidityPresent = !!forecastControl.forecast.lastObservation?.humidity && !!forecastControl.forecast.lastObservation?.humidityUnit;
        if (Plasmoid.configuration.showHumidityInTooltip && isHumidityPresent) {
            let humidity = Util.percentToDisplayString(forecastControl.forecast.lastObservation.humidity);
            tooltips.push(i18n("Humidity: %1", humidity));
        }

        return tooltips.join("\n");
    }

    // Only exists because the default CompactRepresentation doesn't expose:
    // - Icon overlays, or a generic way to overlay something on top of the icon
    // - The ability to show text below or beside the icon
    // TODO remove once it gains those features.
    compactRepresentation: CompactRepresentation {
        metaData: forecastControl.forecast?.metaData
        lastObservation: forecastControl.forecast?.lastObservation
        displayTemperatureUnit: root.displayTemperatureUnit
    }

    fullRepresentation: FullRepresentation {

        status: root.status

        invalidUnit: root.invalidUnit
        displaySpeedUnit: root.displaySpeedUnit
        displayPressureUnit: root.displayPressureUnit
        displayTemperatureUnit: root.displayTemperatureUnit
        displayVisibilityUnit: root.displayVisibilityUnit

        station: forecastControl.forecast?.station
        futureDays: forecastControl.forecast?.futureDays
        warnings: forecastControl.forecast?.warnings
        lastObservation: forecastControl.forecast?.lastObservation
        metaData: forecastControl.forecast?.metaData
    }
}
