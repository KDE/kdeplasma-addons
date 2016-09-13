/***************************************************************************
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *   Copyright (C) 2012 by Luís Gabriel Lima <lampih@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "weatherapplet.h"

#include <KLocalizedString>
#include <KIconLoader>
#include <KUnitConversion/Value>

#include <Plasma/Package>

#include <cmath>

template <typename T>
T clampValue(T value, int decimals)
{
    const T mul = std::pow(static_cast<T>(10), decimals);
    return int(value * mul) / mul;
}

/**
 * Returns the @p iconName if the current icon theme contains an icon with that name,
 * otherwise returns "weather-not-available" (expecting the icon theme to have that in any case).
 */
QString existingWeatherIconName(const QString &iconName)
{
    const bool isValid = !iconName.isEmpty() &&
           !KIconLoader::global()->loadIcon(iconName, KIconLoader::Desktop, 0,
                                            KIconLoader::DefaultState, QStringList(), 0, true).isNull();
    return isValid ? iconName : QStringLiteral("weather-not-available");
}


WeatherApplet::WeatherApplet(QObject *parent, const QVariantList &args)
        : WeatherPopupApplet(parent, args)
{
}

void WeatherApplet::init()
{
    resetPanelModel();

    WeatherPopupApplet::init();
}

WeatherApplet::~WeatherApplet()
{
}

QString WeatherApplet::convertTemperature(KUnitConversion::Unit format, const QVariant &value,
                                          int type, bool rounded, bool degreesOnly)
{
    return convertTemperature(format, value.toFloat(), type, rounded, degreesOnly);
}

QString WeatherApplet::convertTemperature(KUnitConversion::Unit format, float value,
                                          int type, bool rounded, bool degreesOnly)
{
    KUnitConversion::Value v(value, static_cast<KUnitConversion::UnitId>(type));
    v = v.convertTo(format);

    if (rounded) {
        int tempNumber = qRound(v.number());
        if (degreesOnly) {
            return i18nc("temperature, unit", "%1%2", tempNumber, i18nc("Degree, unit symbol", "°"));
        } else {
            return i18nc("temperature, unit", "%1%2", tempNumber, v.unit().symbol());
        }
    } else {
        const QString formattedTemp = QLocale().toString(clampValue(v.number(), 1), 'f', 1);
        if (degreesOnly) {
            return i18nc("temperature, unit", "%1%2", formattedTemp, i18nc("Degree, unit symbol", "°"));
        } else {
            return i18nc("temperature, unit", "%1%2", formattedTemp, v.unit().symbol());
        }
    }
}

bool WeatherApplet::isValidData(const QString &data) const
{
    return (!data.isEmpty() && data != i18n("N/A"));
}

bool WeatherApplet::isValidData(const QVariant &data) const
{
    return isValidData(data.toString());
}

void WeatherApplet::resetPanelModel()
{
    m_panelModel["location"] = "";
    m_panelModel["currentDayLowTemperature"] = "";
    m_panelModel["currentDayHighTemperature"] = "";
    m_panelModel["currentConditions"] = "";
    m_panelModel["currentTemperature"] = "";
    m_panelModel["currentConditionIcon"] = "";
    m_panelModel["totalDays"] = "";
    m_panelModel["courtesy"] = "";
    m_panelModel["creditUrl"] = "";
}

void WeatherApplet::updatePanelModel(const Plasma::DataEngine::Data &data)
{
    resetPanelModel();

    m_panelModel["location"] = data["Place"].toString();

    const int reportTemperatureUnit = data["Temperature Unit"].toInt();
    const KUnitConversion::Unit displayTemperatureUnit = temperatureUnit();

    // Get current time period of day
    const QStringList fiveDayTokens = data["Short Forecast Day 0"].toString().split(QLatin1Char('|'));

    if (fiveDayTokens.count() == 6) {

        const QString& reportLowString = fiveDayTokens[4];
        if (reportLowString != QLatin1String("N/A") && !reportLowString.isEmpty()) {
            m_panelModel["currentDayLowTemperature"] =
                convertTemperature(displayTemperatureUnit, reportLowString, reportTemperatureUnit, true);
        }

        const QString& reportHighString = fiveDayTokens[3];
        if (reportHighString != QLatin1String("N/A") && !reportHighString.isEmpty()) {
            m_panelModel["currentDayHighTemperature"] =
                convertTemperature(displayTemperatureUnit, reportHighString, reportTemperatureUnit, true);
        }
    }

    m_panelModel["currentConditions"] = data["Current Conditions"].toString().trimmed();

    const QVariant temperature = data[QStringLiteral("Temperature")];
    if (isValidData(temperature)) {
        m_panelModel["currentTemperature"] = convertTemperature(displayTemperatureUnit, temperature, reportTemperatureUnit);
    }

    const QString conditionIconName = data["Condition Icon"].toString();
    QString weatherIconName;
    // specific icon?
    if (!conditionIconName.isEmpty() &&
        conditionIconName != QLatin1String("weather-none-available") &&
        conditionIconName != QLatin1String("N/U") && // TODO: N/U and N/A should not be used here, fix dataengines
        conditionIconName != QLatin1String("N/A")) {

        weatherIconName = existingWeatherIconName(conditionIconName);
    } else {
        // icon to use from current weather forecast?
        if (fiveDayTokens.count() == 6 && fiveDayTokens[1] != QLatin1String("N/U")) {
            // show the current weather
            weatherIconName = existingWeatherIconName(fiveDayTokens[1]);
        } else {
            weatherIconName = QStringLiteral("weather-none-available");
        }
    }
    m_panelModel["currentConditionIcon"] = weatherIconName;

    m_panelModel["courtesy"] = data["Credit"].toString();
    m_panelModel["creditUrl"] = data["Credit Url"].toString();
}

void WeatherApplet::updateFiveDaysModel(const Plasma::DataEngine::Data &data)
{
    const int foreCastDayCount = data[QStringLiteral("Total Weather Days")].toInt();
    if (foreCastDayCount <= 0) {
        return;
    }

    m_fiveDaysModel.clear();

    const int reportTemperatureUnit = data[QStringLiteral("Temperature Unit")].toInt();
    const KUnitConversion::Unit displayTemperatureUnit = temperatureUnit();

    QStringList dayItems;
    QStringList conditionItems; // Icon
    QStringList hiItems;
    QStringList lowItems;

    for (int i = 0; i < foreCastDayCount; ++i) {
        const QString foreCastDayKey = QStringLiteral("Short Forecast Day %1").arg(i);
        const QStringList fiveDayTokens = data[foreCastDayKey].toString().split(QLatin1Char('|'));

        if (fiveDayTokens.count() != 6) {
            // We don't have the right number of tokens, abort trying
            break;
        }

        dayItems << fiveDayTokens[0];

        // If we see N/U (Not Used) we skip the item
        const QString& weatherIconName = fiveDayTokens[1];
        if (weatherIconName != QLatin1String("N/U") && !weatherIconName.isEmpty()) {
            QString iconAndToolTip = existingWeatherIconName(weatherIconName);

            iconAndToolTip += QLatin1Char('|');

            const QString& condition = fiveDayTokens[2];
            const QString& probability = fiveDayTokens[5];
            if (probability != QLatin1String("N/U") &&
                probability != QLatin1String("N/A") &&
                !probability.isEmpty()) {
                iconAndToolTip += i18nc("certain weather condition (probability percentage)",
                                        "%1 (%2%)", condition, probability);
            } else {
                iconAndToolTip += condition;
            }
            conditionItems << iconAndToolTip;
        }

        const QString& tempHigh = fiveDayTokens[3];
        if (tempHigh != QLatin1String("N/U")) {
            if (tempHigh == QLatin1String("N/A") || tempHigh.isEmpty()) {
                hiItems << i18nc("Short for no data available", "-");
            } else {
                hiItems << convertTemperature(displayTemperatureUnit,
                                              tempHigh,
                                              reportTemperatureUnit,
                                              true);
            }
        }

        const QString& tempLow = fiveDayTokens[4];
        if (tempLow != QLatin1String("N/U")) {
            if (tempLow == QLatin1String("N/A") || tempLow.isEmpty()) {
                lowItems << i18nc("Short for no data available", "-");
            } else {
                lowItems << convertTemperature(displayTemperatureUnit,
                                               tempLow,
                                               reportTemperatureUnit,
                                               true);
            }
        }
    }

    if (!dayItems.isEmpty()) {
        m_fiveDaysModel << dayItems;
    }
    if (!conditionItems.isEmpty()) {
        m_fiveDaysModel << conditionItems;
    }
    if (!hiItems.isEmpty())  {
        m_fiveDaysModel << hiItems;
    }
    if (!lowItems.isEmpty()) {
        m_fiveDaysModel << lowItems;
    }

    m_panelModel["totalDays"] = i18ncp("Forecast period timeframe", "1 Day",
                                       "%1 Days", foreCastDayCount);
}

void WeatherApplet::updateDetailsModel(const Plasma::DataEngine::Data &data)
{
    m_detailsModel.clear();

    QLocale locale;
    const QString textId = QStringLiteral("text");
    const QString iconId = QStringLiteral("icon");

    // reused map for each row
    QVariantMap row;
    row.insert(iconId, QString());
    row.insert(textId, QString());

    const int reportTemperatureUnit = data[QStringLiteral("Temperature Unit")].toInt();
    const KUnitConversion::Unit displayTemperatureUnit = temperatureUnit();

    const QVariant windChill = data[QStringLiteral("Windchill")];
    if (isValidData(windChill)) {
        // Use temperature unit to convert windchill temperature
        // we only show degrees symbol not actual temperature unit
        const QString temp = convertTemperature(displayTemperatureUnit, windChill, reportTemperatureUnit, false, true);
        row[textId] = i18nc("windchill, unit", "Windchill: %1", temp);

        m_detailsModel << row;
    }

    const QString humidex = data[QStringLiteral("Humidex")].toString();
    if (isValidData(humidex)) {
        // TODO: this seems wrong, does the humidex have temperature as units?
        // Use temperature unit to convert humidex temperature
        // we only show degrees symbol not actual temperature unit
        QString temp = convertTemperature(displayTemperatureUnit, humidex, reportTemperatureUnit, false, true);
        row[textId] = i18nc("humidex, unit","Humidex: %1", temp);

        m_detailsModel << row;
    }

    const QVariant dewpoint = data[QStringLiteral("Dewpoint")];
    if (isValidData(dewpoint)) {
        QString temp = convertTemperature(displayTemperatureUnit, dewpoint, reportTemperatureUnit);
        row[textId] = i18nc("ground temperature, unit", "Dewpoint: %1", temp);

        m_detailsModel << row;
    }

    const QVariant pressure = data[QStringLiteral("Pressure")];
    if (isValidData(pressure)) {
        KUnitConversion::Value v(pressure.toDouble(),
                                 static_cast<KUnitConversion::UnitId>(data["Pressure Unit"].toInt()));
        v = v.convertTo(pressureUnit());
        row[textId] = i18nc("pressure, unit","Pressure: %1 %2",
                            locale.toString(clampValue(v.number(), 2), 'f', 2), v.unit().symbol());

        m_detailsModel << row;
    }

    const QString pressureTendency = data["Pressure Tendency"].toString();
    if (isValidData(pressureTendency)) {
        const QString i18nPressureTendency = i18nc("pressure tendency", pressureTendency.toUtf8().data());
        row[textId] = i18nc("pressure tendency, rising/falling/steady",
                            "Pressure Tendency: %1", i18nPressureTendency);

        m_detailsModel << row;
    }

    const QVariant visibility = data[QStringLiteral("Visibility")];
    if (isValidData(visibility)) {
        const KUnitConversion::UnitId unitId = static_cast<KUnitConversion::UnitId>(data["Visibility Unit"].toInt());
        if (unitId != KUnitConversion::NoUnit) {
            KUnitConversion::Value v(visibility.toDouble(), unitId);
            v = v.convertTo(visibilityUnit());
            row[textId] = i18nc("distance, unit","Visibility: %1 %2",
                                locale.toString(clampValue(v.number(), 1), 'f', 1), v.unit().symbol());
        } else {
            row[textId] = i18nc("visibility from distance", "Visibility: %1", visibility.toString());
        }

        m_detailsModel << row;
    }

    const QVariant humidity = data[QStringLiteral("Humidity")];
    if (isValidData(humidity)) {
        row[textId] = i18nc("content of water in air", "Humidity: %1%2",
                            locale.toString(clampValue(humidity.toFloat(), 0), 'f', 0), i18nc("Percent, measure unit", "%"));

        m_detailsModel << row;
    }

    const QVariant windSpeed = data[QStringLiteral("Wind Speed")];
    if (isValidData(windSpeed)) {
        // TODO: missing check for windDirection validness
        const QString windDirection = data["Wind Direction"].toString();
        row[iconId] = windDirection;

        bool isNumeric;
        const double windSpeedNumeric = windSpeed.toDouble(&isNumeric);
        if (isNumeric) {
            if (windSpeedNumeric != 0) {
                KUnitConversion::Value v(windSpeedNumeric,
                                        static_cast<KUnitConversion::UnitId>(data["Wind Speed Unit"].toInt()));
                v = v.convertTo(speedUnit());
                const QString i18nWindDirection = i18nc("wind direction", windDirection.toUtf8().data());
                row[textId] = i18nc("wind direction, speed","%1 %2 %3", i18nWindDirection,
                                    locale.toString(clampValue(v.number(), 1), 'f', 1), v.unit().symbol());
            } else {
                row[textId] = i18nc("Wind condition", "Calm");
            }
        } else {
            row[textId] = windSpeed.toString();
        }

        m_detailsModel << row;
        row[iconId] = QString(); // reset
    }

    const QVariant windGust = data[QStringLiteral("Wind Gust")];
    if (isValidData(windGust)) {
        // Convert the wind format for nonstandard types
        KUnitConversion::Value v(windGust.toDouble(),
                                 static_cast<KUnitConversion::UnitId>(data["Wind Speed Unit"].toInt()));
        v = v.convertTo(speedUnit());
        row[textId] = i18nc("winds exceeding wind speed briefly", "Wind Gust: %1 %2",
                            locale.toString(clampValue(v.number(), 1), 'f', 1), v.unit().symbol());

        m_detailsModel << row;
    }
}

void WeatherApplet::updateNoticesModel(const Plasma::DataEngine::Data &data)
{
    m_noticesModel.clear();

    QVariantList warnings;
    for (int i = 0; i < data["Total Warnings Issued"].toInt(); ++i) {
        QVariantMap warning;
        warning["description"] = data[QStringLiteral("Warning Description %1").arg(i)];
        warning["info"] = data[QStringLiteral("Warning Info %1").arg(i)];
        warnings << warning;
    }
    m_noticesModel << QVariant(warnings);

    QVariantList watches;
    for (int i = 0; i < data["Total Watches Issued"].toInt(); ++i) {
        QVariantMap watch;
        watch["description"] = data[QStringLiteral("Watch Description %1").arg(i)];
        watch["info"] = data[QStringLiteral("Watch Info %1").arg(i)];
        watches << watch;
    }
    m_noticesModel << QVariant(watches);
}

void WeatherApplet::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (data.isEmpty()) {
        return;
    }

    updatePanelModel(data);
    updateFiveDaysModel(data);
    updateDetailsModel(data);
    updateNoticesModel(data);
    WeatherPopupApplet::dataUpdated(source, data);

    emit modelUpdated();
}

void WeatherApplet::saveConfig(const QVariantMap& configChanges)
{
    // TODO: if just units where changed there is no need to reset the complete model or reconnect to engine
    resetPanelModel();
    m_fiveDaysModel.clear();
    m_detailsModel.clear();

    emit modelUpdated();

    WeatherPopupApplet::saveConfig(configChanges);
}

K_EXPORT_PLASMA_APPLET_WITH_JSON(weather, WeatherApplet, "metadata.json")

#include "weatherapplet.moc"
