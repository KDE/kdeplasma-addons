/*
    SPDX-FileCopyrightText: 2009 Thilo-Alexander Ginkel <thilo@ginkel.com>

    Based upon BBC Weather Ion by Shawn Starr
    SPDX-FileCopyrightText: 2007-2009 Shawn Starr <shawn.starr@rogers.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/* Ion for weather data from wetter.com */

// Sample URLs:
// https://api.wetter.com/location/index/search/Heidelberg/project/weatherion/cs/9090dec6e783b96bd6a6ca9d451f3fee
// https://api.wetter.com/forecast/weather/city/DE0004329/project/weatherion/cs/89f1264869cce5c6fd5a2db80051f3d8

#include "ion_wettercom.h"

#include <KIO/TransferJob>
#include <KLocalizedString>
#include <KUnitConversion/Converter>

#include <KPluginFactory>
#include <QCryptographicHash>
#include <QLocale>
#include <QUrl>
#include <QXmlStreamReader>

#include "wettercom_debug.h"

#include <QTimeZone>
#include <qtimezone.h>

K_PLUGIN_CLASS_WITH_JSON(WetterComIon, "metadata.json")

WetterComIon::WetterComIon(QObject *parent, const QVariantList &args)
    : Ion(parent)
{
    Q_UNUSED(args);
}

WetterComIon::~WetterComIon()
{
}

QMap<QString, Ion::ConditionIcons> WetterComIon::setupCommonIconMappings() const
{
    return QMap<QString, ConditionIcons>{
        {QStringLiteral("3"), Overcast},
        {QStringLiteral("30"), Overcast},
        {QStringLiteral("4"), Haze},
        {QStringLiteral("40"), Haze},
        {QStringLiteral("45"), Haze},
        {QStringLiteral("48"), Haze},
        {QStringLiteral("49"), Haze},
        {QStringLiteral("5"), Mist},
        {QStringLiteral("50"), Mist},
        {QStringLiteral("51"), Mist},
        {QStringLiteral("53"), Mist},
        {QStringLiteral("55"), Mist},
        {QStringLiteral("56"), FreezingDrizzle},
        {QStringLiteral("57"), FreezingDrizzle},
        {QStringLiteral("6"), Rain},
        {QStringLiteral("60"), LightRain},
        {QStringLiteral("61"), LightRain},
        {QStringLiteral("63"), Rain},
        {QStringLiteral("65"), Rain},
        {QStringLiteral("66"), FreezingRain},
        {QStringLiteral("67"), FreezingRain},
        {QStringLiteral("68"), RainSnow},
        {QStringLiteral("69"), RainSnow},
        {QStringLiteral("7"), Snow},
        {QStringLiteral("70"), LightSnow},
        {QStringLiteral("71"), LightSnow},
        {QStringLiteral("73"), Snow},
        {QStringLiteral("75"), Flurries},
        {QStringLiteral("8"), Showers},
        {QStringLiteral("81"), Showers},
        {QStringLiteral("82"), Showers},
        {QStringLiteral("83"), RainSnow},
        {QStringLiteral("84"), RainSnow},
        {QStringLiteral("85"), Snow},
        {QStringLiteral("86"), Snow},
        {QStringLiteral("9"), Thunderstorm},
        {QStringLiteral("90"), Thunderstorm},
        {QStringLiteral("96"), Thunderstorm},
        {QStringLiteral("999"), NotAvailable},
    };
}

QMap<QString, Ion::ConditionIcons> WetterComIon::setupDayIconMappings() const
{
    QMap<QString, ConditionIcons> conditionList = setupCommonIconMappings();

    conditionList.insert(QStringLiteral("0"), ClearDay);
    conditionList.insert(QStringLiteral("1"), FewCloudsDay);
    conditionList.insert(QStringLiteral("10"), FewCloudsDay);
    conditionList.insert(QStringLiteral("2"), PartlyCloudyDay);
    conditionList.insert(QStringLiteral("20"), PartlyCloudyDay);
    conditionList.insert(QStringLiteral("80"), ChanceShowersDay);
    conditionList.insert(QStringLiteral("95"), ChanceThunderstormDay);

    return conditionList;
}

QMap<QString, Ion::ConditionIcons> const &WetterComIon::dayIcons() const
{
    static QMap<QString, ConditionIcons> const val = setupDayIconMappings();
    return val;
}

QMap<QString, Ion::ConditionIcons> WetterComIon::setupNightIconMappings() const
{
    QMap<QString, ConditionIcons> conditionList = setupCommonIconMappings();

    conditionList.insert(QStringLiteral("0"), ClearNight);
    conditionList.insert(QStringLiteral("1"), FewCloudsNight);
    conditionList.insert(QStringLiteral("10"), FewCloudsNight);
    conditionList.insert(QStringLiteral("2"), PartlyCloudyNight);
    conditionList.insert(QStringLiteral("20"), PartlyCloudyNight);
    conditionList.insert(QStringLiteral("80"), ChanceShowersNight);
    conditionList.insert(QStringLiteral("95"), ChanceThunderstormNight);

    return conditionList;
}

QMap<QString, Ion::ConditionIcons> const &WetterComIon::nightIcons() const
{
    static QMap<QString, ConditionIcons> const val = setupNightIconMappings();
    return val;
}

QHash<QString, QString> WetterComIon::setupCommonConditionMappings() const
{
    return QHash<QString, QString>{
        {QStringLiteral("1"), i18nc("weather condition", "few clouds")},
        {QStringLiteral("10"), i18nc("weather condition", "few clouds")},
        {QStringLiteral("2"), i18nc("weather condition", "cloudy")},
        {QStringLiteral("20"), i18nc("weather condition", "cloudy")},
        {QStringLiteral("3"), i18nc("weather condition", "overcast")},
        {QStringLiteral("30"), i18nc("weather condition", "overcast")},
        {QStringLiteral("4"), i18nc("weather condition", "haze")},
        {QStringLiteral("40"), i18nc("weather condition", "haze")},
        {QStringLiteral("45"), i18nc("weather condition", "haze")},
        {QStringLiteral("48"), i18nc("weather condition", "fog with icing")},
        {QStringLiteral("49"), i18nc("weather condition", "fog with icing")},
        {QStringLiteral("5"), i18nc("weather condition", "drizzle")},
        {QStringLiteral("50"), i18nc("weather condition", "drizzle")},
        {QStringLiteral("51"), i18nc("weather condition", "light drizzle")},
        {QStringLiteral("53"), i18nc("weather condition", "drizzle")},
        {QStringLiteral("55"), i18nc("weather condition", "heavy drizzle")},
        {QStringLiteral("56"), i18nc("weather condition", "freezing drizzle")},
        {QStringLiteral("57"), i18nc("weather condition", "heavy freezing drizzle")},
        {QStringLiteral("6"), i18nc("weather condition", "rain")},
        {QStringLiteral("60"), i18nc("weather condition", "light rain")},
        {QStringLiteral("61"), i18nc("weather condition", "light rain")},
        {QStringLiteral("63"), i18nc("weather condition", "moderate rain")},
        {QStringLiteral("65"), i18nc("weather condition", "heavy rain")},
        {QStringLiteral("66"), i18nc("weather condition", "light freezing rain")},
        {QStringLiteral("67"), i18nc("weather condition", "freezing rain")},
        {QStringLiteral("68"), i18nc("weather condition", "light rain snow")},
        {QStringLiteral("69"), i18nc("weather condition", "heavy rain snow")},
        {QStringLiteral("7"), i18nc("weather condition", "snow")},
        {QStringLiteral("70"), i18nc("weather condition", "light snow")},
        {QStringLiteral("71"), i18nc("weather condition", "light snow")},
        {QStringLiteral("73"), i18nc("weather condition", "moderate snow")},
        {QStringLiteral("75"), i18nc("weather condition", "heavy snow")},
        {QStringLiteral("8"), i18nc("weather condition", "showers")},
        {QStringLiteral("80"), i18nc("weather condition", "light showers")},
        {QStringLiteral("81"), i18nc("weather condition", "showers")},
        {QStringLiteral("82"), i18nc("weather condition", "heavy showers")},
        {QStringLiteral("83"), i18nc("weather condition", "light snow rain showers")},
        {QStringLiteral("84"), i18nc("weather condition", "heavy snow rain showers")},
        {QStringLiteral("85"), i18nc("weather condition", "light snow showers")},
        {QStringLiteral("86"), i18nc("weather condition", "snow showers")},
        {QStringLiteral("9"), i18nc("weather condition", "thunderstorm")},
        {QStringLiteral("90"), i18nc("weather condition", "thunderstorm")},
        {QStringLiteral("95"), i18nc("weather condition", "light thunderstorm")},
        {QStringLiteral("96"), i18nc("weather condition", "heavy thunderstorm")},
        {QStringLiteral("999"), i18nc("weather condition", "n/a")},
    };
}

QHash<QString, QString> WetterComIon::setupDayConditionMappings() const
{
    QHash<QString, QString> conditionList = setupCommonConditionMappings();
    conditionList.insert(QStringLiteral("0"), i18nc("weather condition", "sunny"));
    return conditionList;
}

QHash<QString, QString> const &WetterComIon::dayConditions() const
{
    static QHash<QString, QString> const val = setupDayConditionMappings();
    return val;
}

QHash<QString, QString> WetterComIon::setupNightConditionMappings() const
{
    QHash<QString, QString> conditionList = setupCommonConditionMappings();
    conditionList.insert(QStringLiteral("0"), i18nc("weather condition", "clear sky"));
    return conditionList;
}

QHash<QString, QString> const &WetterComIon::nightConditions() const
{
    static QHash<QString, QString> const val = setupNightConditionMappings();
    return val;
}

QString WetterComIon::getWeatherCondition(const QHash<QString, QString> &conditionList, const QString &condition) const
{
    return conditionList[condition];
}

/*
 * Handling of place searches
 */
void WetterComIon::findPlaces(std::shared_ptr<QPromise<std::shared_ptr<Locations>>> promise, const QString &searchString)
{
    qCDebug(WEATHER::ION::WETTERCOM) << "Begin search places for search string" << searchString;

    m_locationPromise = promise;

    m_locationPromise->start();

    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(QByteArray(PROJECTNAME));
    md5.addData(QByteArray(APIKEY));
    md5.addData(searchString.toUtf8());
    const QString encodedKey = QString::fromLatin1(md5.result().toHex());

    const QUrl url(QStringLiteral(SEARCH_URL).arg(searchString, encodedKey));
    qCDebug(WEATHER::ION::WETTERCOM) << "Search URL for" << searchString << url;

    auto job = KIO::get(url, KIO::Reload, KIO::HideProgressInfo);
    job->addMetaData(QStringLiteral("cookies"), QStringLiteral("none")); // Disable displaying cookies
    m_jobXml = std::make_unique<QXmlStreamReader>();

    connect(job, &KIO::TransferJob::data, this, &WetterComIon::places_slotDataArrived);
    connect(job, &KJob::result, this, &WetterComIon::places_slotJobFinished);
}

void WetterComIon::places_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job);

    QByteArray local = data;

    if (m_locationPromise->isCanceled()) {
        qCDebug(WEATHER::ION::WETTERCOM) << "Fetch locations cancelled";
        return;
    }

    if (data.isEmpty() || !m_jobXml) {
        qCDebug(WEATHER::ION::WETTERCOM) << "Location data is empty. Return";
        return;
    }

    m_jobXml->addData(local);
}

void WetterComIon::places_slotJobFinished(KJob *job)
{
    if (m_locationPromise->isCanceled()) {
        qCDebug(WEATHER::ION::WETTERCOM) << "Fetch locations cancelled";
        m_jobXml.reset();
        m_locationPromise.reset();
        return;
    }

    if (job->error() == KIO::ERR_SERVER_TIMEOUT) {
        qCDebug(WEATHER::ION::WETTERCOM) << "Error: " << job->errorString();
        m_locationPromise->finish();
        m_jobXml.reset();
        m_locationPromise.reset();
        return;
    }

    if (m_jobXml) {
        parseSearchResults(*m_jobXml);
    }

    qCDebug(WEATHER::ION::WETTERCOM) << "Fetch locations ended";

    m_jobXml.reset();
    m_locationPromise->finish();
    m_locationPromise.reset();
}

void WetterComIon::parseSearchResults(QXmlStreamReader &xml)
{
    QString name, code, quarter, state, country;

    QList<PlaceInfo> locations;

    while (!xml.atEnd()) {
        if (m_locationPromise->isCanceled()) {
            return;
        }

        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement()) {
            if (elementName == QLatin1String("search")) {
                break;
            } else if (elementName == QLatin1String("item")) {
                // we parsed a place from the search result
                QString placeName;

                if (quarter.isEmpty()) {
                    placeName = i18nc("Geographical location: city, state, ISO-country-code", "%1, %2, %3", name, state, country);
                } else {
                    placeName = i18nc("Geographical location: quarter (city), state, ISO-country-code", "%1 (%2), %3, %4", quarter, name, state, country);
                }

                qCDebug(WEATHER::ION::WETTERCOM) << "Storing place data for place:" << placeName << "with code" << code;

                PlaceInfo place;
                place.displayName = placeName;
                place.station = name;
                place.placeCode = code;
                locations.append(place);

                name.clear();
                code.clear();
                quarter.clear();
                country.clear();
                state.clear();
            }
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("name")) {
                name = xml.readElementText();
            } else if (elementName == QLatin1String("city_code")) {
                code = xml.readElementText();
            } else if (elementName == QLatin1String("quarter")) {
                quarter = xml.readElementText();
            } else if (elementName == QLatin1String("adm_1_code")) {
                country = xml.readElementText();
            } else if (elementName == QLatin1String("adm_2_name")) {
                state = xml.readElementText();
            }
        }
    }

    if (!locations.count() || xml.error() != QXmlStreamReader::NoError) {
        qCDebug(WEATHER::ION::WETTERCOM) << "Invalid place";
        return;
    }

    updatePlaces(locations);
}

void WetterComIon::updatePlaces(const QList<PlaceInfo> &locations)
{
    if (m_locationPromise->isCanceled()) {
        qCDebug(WEATHER::ION::WETTERCOM) << "Wetter.com: fetch locations cancelled";
        return;
    }

    auto returnLocations = std::make_shared<Locations>();

    for (const auto &rawLocation : std::as_const(locations)) {
        // Extra data format: placeCode;displayName
        Location location;
        location.setCode(rawLocation.placeCode);
        location.setDisplayName(rawLocation.displayName);
        location.setStation(rawLocation.station);
        location.setPlaceInfo(QStringLiteral("place|%1|extra|%2;%3").arg(rawLocation.displayName, rawLocation.placeCode, rawLocation.station));

        returnLocations->addLocation(location);
    }

    qCDebug(WEATHER::ION::WETTERCOM) << "Returning place list";

    m_locationPromise->addResult(returnLocations);
}

/*
 * Handling of forecasts
 */

void WetterComIon::fetchForecast(std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> promise, const QString &placeInfo)
{
    m_forecastPromise = promise;

    m_forecastPromise->start();

    if (m_forecastPromise->isCanceled()) {
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    //"place|%1|extra|%2;%3"
    const QStringList placeAction = placeInfo.split(QLatin1Char('|'));

    if (placeAction.size() == 4 && placeAction[1] == QLatin1String("weather") && !m_isLegacy) {
        m_isLegacy = true;
        m_newPlaceInfo = QLatin1String("place|") + placeAction[2] + QLatin1String("|extra|") + placeAction[3];
        fetchForecast(m_forecastPromise, m_newPlaceInfo);
        return;
    }

    if (placeAction.count() != 4) {
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    qCDebug(WEATHER::ION::WETTERCOM) << "Fetching weather for placeInfo:" << placeInfo;

    qCDebug(WEATHER::ION::WETTERCOM) << "The placeAction is: " << placeAction;

    if (placeAction.count() < 4) {
        qCDebug(WEATHER::ION::WETTERCOM) << "Invalid placeInfo";
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    if (placeAction[3].isEmpty()) {
        qCDebug(WEATHER::ION::WETTERCOM) << "Invalid placeInfo";
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    // Extra data format: placeCode;displayName
    const QStringList extraData = placeAction[3].split(QLatin1Char(';'));
    if (extraData.count() != 2) {
        qCDebug(WEATHER::ION::WETTERCOM) << "Invalid placeInfo";
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    const QString &placeCode = extraData[0];
    m_place.displayName = placeAction[1];
    m_place.placeCode = placeCode;
    m_place.station = extraData[1];

    qCDebug(WEATHER::ION::WETTERCOM) << "About to retrieve forecast for place: " << placeCode << placeAction[2];

    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(QByteArray(PROJECTNAME));
    md5.addData(QByteArray(APIKEY));
    md5.addData(m_place.placeCode.toUtf8());
    const QString encodedKey = QString::fromLatin1(md5.result().toHex());

    const QUrl url(QStringLiteral(FORECAST_URL).arg(m_place.placeCode, encodedKey));
    qCDebug(WEATHER::ION::WETTERCOM) << "Fetch Weather URL for" << m_place.displayName << url;

    auto job = KIO::get(url, KIO::Reload, KIO::HideProgressInfo);
    job->addMetaData(QStringLiteral("cookies"), QStringLiteral("none"));
    m_jobXml = std::make_unique<QXmlStreamReader>();

    connect(job, &KIO::TransferJob::data, this, &WetterComIon::forecast_slotDataArrived);
    connect(job, &KJob::result, this, &WetterComIon::forecast_slotJobFinished);
}

void WetterComIon::forecast_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job);

    QByteArray local = data;

    if (m_forecastPromise->isCanceled()) {
        qCDebug(WEATHER::ION::WETTERCOM) << "Fetch forecast cancelled";
        return;
    }

    if (data.isEmpty() || !m_jobXml) {
        qCDebug(WEATHER::ION::WETTERCOM) << "Forecast data is empty. Return";
        return;
    }

    m_jobXml->addData(local);
}

void WetterComIon::forecast_slotJobFinished(KJob *job)
{
    if (m_forecastPromise->isCanceled()) {
        qCDebug(WEATHER::ION::WETTERCOM) << "Fetch forecast cancelled";
        m_forecastPromise->finish();
        m_jobXml.reset();
        m_forecastPromise.reset();
        return;
    }

    if (job->error() == KIO::ERR_SERVER_TIMEOUT) {
        qCDebug(WEATHER::ION::WETTERCOM) << "Error: " << job->errorString();
        m_forecastPromise->finish();
        m_jobXml.reset();
        m_forecastPromise.reset();
        return;
    }

    if (m_jobXml) {
        parseWeatherForecast(*m_jobXml);
    }

    qCDebug(WEATHER::ION::WETTERCOM) << "Fetch forecast ended";

    m_jobXml.reset();
    m_forecastPromise->finish();
    m_forecastPromise.reset();
}

void WetterComIon::parseWeatherForecast(QXmlStreamReader &xml)
{
    qCDebug(WEATHER::ION::WETTERCOM) << "About to parse forecast for source:" << m_place.displayName;

    m_weatherData.dayForecasts.clear();
    m_weatherData.hourlyForecasts.clear();

    struct ForecastData {
        int weather = -1;
        int probability = 0;
        int tempMax = -273;
        int tempMin = 100;
        uint utcTime = 0;
        uint localTime = 0;
    };

    auto appendDayForecast = [this](const ForecastData &data) {
        if (data.weather < 0 || data.utcTime == 0) {
            return;
        }

        auto forecast = std::make_shared<WeatherData::Forecast>();

        forecast->period = QDateTime::fromSecsSinceEpoch(data.utcTime, QTimeZone::LocalTime);

        const QString weatherString = QString::number(data.weather);

        forecast->iconName = getWeatherIcon(dayIcons(), weatherString);
        forecast->summary = getWeatherCondition(dayConditions(), weatherString);
        forecast->tempHigh = data.tempMax;
        forecast->tempLow = data.tempMin;
        forecast->probability = data.probability;

        m_weatherData.dayForecasts.append(forecast);
    };

    auto appendHourlyForecast = [this](const ForecastData &data) {
        if (data.weather < 0 || data.utcTime == 0) {
            return;
        }

        auto forecast = std::make_shared<WeatherData::Forecast>();

        const QString weatherString = QString::number(data.weather);

        // wetter.com provides both UTC and local timestamps
        const auto forecastTimezone = QTimeZone::fromSecondsAheadOfUtc(data.localTime - data.utcTime);
        const auto localDateTime = QDateTime::fromSecsSinceEpoch(data.utcTime, QTimeZone::UTC).toTimeZone(forecastTimezone);
        const int hour = localDateTime.time().hour();

        // TODO use sunrise/sunset instead
        const bool isDay = hour > 6 && hour < 20;

        if (isDay) {
            forecast->iconName = getWeatherIcon(dayIcons(), weatherString);
            forecast->summary = getWeatherCondition(dayConditions(), weatherString);
        } else {
            forecast->iconName = getWeatherIcon(nightIcons(), weatherString);
            forecast->summary = getWeatherCondition(nightConditions(), weatherString);
        }

        forecast->period = localDateTime;

        forecast->tempHigh = data.tempMax;
        forecast->tempLow = data.tempMin;
        forecast->probability = data.probability;

        m_weatherData.hourlyForecasts.append(forecast);
    };

    ForecastData dayData;
    ForecastData hourlyData;

    bool insideTime = false;

    m_weatherData.place = m_place.displayName;

    while (!xml.atEnd()) {
        if (m_forecastPromise->isCanceled()) {
            qCDebug(WEATHER::ION::WETTERCOM) << "Fetch forecast cancelled";
            return;
        }

        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("time")) {
                insideTime = true;
                hourlyData = {};
                continue;
            }

            if (elementName == QLatin1String("tx")) {
                const int value = qRound(xml.readElementText().toDouble());

                if (insideTime) {
                    hourlyData.tempMax = value;
                } else {
                    dayData.tempMax = value;
                }
            } else if (elementName == QLatin1String("tn")) {
                const int value = qRound(xml.readElementText().toDouble());

                if (insideTime) {
                    hourlyData.tempMin = value;
                } else {
                    dayData.tempMin = value;
                }
            } else if (elementName == QLatin1Char('w')) {
                const int value = xml.readElementText().toInt();

                if (insideTime) {
                    hourlyData.weather = value;
                } else {
                    dayData.weather = value;
                }
            } else if (elementName == QLatin1String("pc")) {
                const int value = xml.readElementText().toInt();

                if (insideTime) {
                    hourlyData.probability = value;
                } else {
                    dayData.probability = value;
                }
            } else if (elementName == QLatin1String("du")) {
                const uint value = xml.readElementText().toUInt();

                if (insideTime) {
                    hourlyData.utcTime = value;
                } else {
                    dayData.utcTime = value;
                }
            } else if (elementName == QLatin1Char('d')) {
                if (insideTime) {
                    hourlyData.localTime = xml.readElementText().toUInt();
                }
            } else if (elementName == QLatin1String("name")) {
                m_weatherData.stationName = xml.readElementText();
            } else if (elementName == QLatin1String("text")) {
                m_weatherData.credits = xml.readElementText();
            } else if (elementName == QLatin1String("link")) {
                m_weatherData.creditsUrl = xml.readElementText();
            }
        }

        if (xml.isEndElement()) {
            if (elementName == QLatin1String("time")) {
                appendHourlyForecast(hourlyData);
                insideTime = false;
            } else if (elementName == QLatin1String("date")) {
                appendDayForecast(dayData);
                dayData = {};
            } else if (elementName == QLatin1String("city")) {
                break;
            }
        }
    }

    if (xml.hasError()) {
        qCWarning(WEATHER::ION::WETTERCOM) << "XML error while parsing forecast:" << xml.errorString();
    }

    updateWeather();
}

void WetterComIon::updateWeather()
{
    if (m_forecastPromise->isCanceled()) {
        qCDebug(WEATHER::ION::WETTERCOM) << "Fetch forecast cancelled";
        return;
    }

    qCDebug(WEATHER::ION::WETTERCOM) << "Update forecast for place code:" << m_place.placeCode << ". Total day forecasts: " << m_weatherData.dayForecasts.size()
                                     << ". Total hourly forecasts: " << m_weatherData.hourlyForecasts.size();

    auto forecast = std::make_shared<Forecast>();

    if (!m_weatherData.dayForecasts.isEmpty() && !m_weatherData.hourlyForecasts.isEmpty()) {
        Station station;

        station.setPlace(m_place.displayName);
        station.setStation(m_place.station);

        if (m_isLegacy) {
            m_isLegacy = false;
            station.setNewPlaceInfo(m_newPlaceInfo);
        }

        MetaData metaData;
        metaData.setCredit(m_weatherData.credits); // FIXME i18n?
        metaData.setCreditURL(QStringLiteral("%1/place/%2.html").arg(m_weatherData.creditsUrl).arg(m_place.placeCode));
        metaData.setTemperatureUnit(KUnitConversion::Celsius);

        forecast->setStation(station);
        forecast->setMetadata(metaData);

        std::shared_ptr<FutureDays> nextDays = std::make_shared<FutureDays>();

        for (auto dayForecast : m_weatherData.dayForecasts) {
            FutureDayForecast nextDay(dayForecast->period);

            FutureForecast futureForecast;
            futureForecast.setConditionIcon(dayForecast->iconName);
            futureForecast.setCondition(dayForecast->summary);
            futureForecast.setHighTemp(dayForecast->tempHigh);
            futureForecast.setLowTemp(dayForecast->tempLow);
            futureForecast.setConditionProbability(dayForecast->probability);

            nextDay.setDaytime(futureForecast);

            nextDays->addDay(nextDay);
        }

        forecast->setFutureDays(nextDays);

        std::shared_ptr<FutureHours> nextHours = std::make_shared<FutureHours>();

        for (auto hourlyForecast : m_weatherData.hourlyForecasts) {
            FutureHourForecast nextHour(hourlyForecast->period);

            nextHour.setConditionIcon(hourlyForecast->iconName);
            nextHour.setCondition(hourlyForecast->summary);
            nextHour.setHighTemp(hourlyForecast->tempHigh);
            nextHour.setLowTemp(hourlyForecast->tempLow);
            nextHour.setConditionProbability(hourlyForecast->probability);

            nextHours->addHour(nextHour);
        }

        forecast->setFutureHours(nextHours);

        qCDebug(WEATHER::ION::WETTERCOM) << "Ended fill forecast for :" << m_place.displayName;
    } else {
        forecast->setError();
        qCDebug(WEATHER::ION::WETTERCOM) << "Something went wrong when parsing weather data for place with code:" << m_place.displayName;
    }
    m_forecastPromise->addResult(forecast);

    m_weatherData.dayForecasts.clear();
    m_weatherData.hourlyForecasts.clear();
}

#include "ion_wettercom.moc"
#include "moc_ion_wettercom.cpp"
