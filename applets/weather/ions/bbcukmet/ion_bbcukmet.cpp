/*
     SPDX-FileCopyrightText: 2007-2009 Shawn Starr <shawn.starr@rogers.com>
     SPDX-FileCopyrightText: 2024 Ismael Asensio <isma.af@gmail.com>

     SPDX-License-Identifier: GPL-2.0-or-later
 */

/* Ion for BBC's Weather from the UK Met Office */

#include "ion_bbcukmet.h"

#include "bbcukmet_debug.h"

#include <KIO/TransferJob>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KUnitConversion/Converter>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimeZone>
#include <QTimer>

K_PLUGIN_CLASS_WITH_JSON(UKMETIon, "metadata.json")

using namespace Qt::StringLiterals;

UKMETIon::UKMETIon(QObject *parent, const QVariantList &args)
    : Ion(parent)

{
    Q_UNUSED(args);
}

UKMETIon::~UKMETIon()
{
}

QMap<QString, Ion::ConditionIcons> UKMETIon::setupDayIconMappings() const
{
    //    ClearDay, FewCloudsDay, PartlyCloudyDay, Overcast,
    //    Showers, ScatteredShowers, Thunderstorm, Snow,
    //    FewCloudsNight, PartlyCloudyNight, ClearNight,
    //    Mist, NotAvailable

    return QMap<QString, ConditionIcons>{
        {QStringLiteral("sunny"), ClearDay},
        //    { QStringLiteral("sunny night"), ClearNight },
        {QStringLiteral("clear"), ClearDay},
        {QStringLiteral("clear sky"), ClearDay},
        {QStringLiteral("sunny intervals"), PartlyCloudyDay},
        //    { QStringLiteral("sunny intervals night"), ClearNight },
        {QStringLiteral("light cloud"), PartlyCloudyDay},
        {QStringLiteral("partly cloudy"), PartlyCloudyDay},
        {QStringLiteral("cloudy"), PartlyCloudyDay},
        {QStringLiteral("white cloud"), PartlyCloudyDay},
        {QStringLiteral("grey cloud"), Overcast},
        {QStringLiteral("thick cloud"), Overcast},
        //    { QStringLiteral("low level cloud"), NotAvailable },
        //    { QStringLiteral("medium level cloud"), NotAvailable },
        //    { QStringLiteral("sandstorm"), NotAvailable },
        {QStringLiteral("drizzle"), LightRain},
        {QStringLiteral("misty"), Mist},
        {QStringLiteral("mist"), Mist},
        {QStringLiteral("fog"), Mist},
        {QStringLiteral("foggy"), Mist},
        {QStringLiteral("tropical storm"), Thunderstorm},
        {QStringLiteral("hazy"), Mist},
        {QStringLiteral("light shower"), Showers},
        {QStringLiteral("light rain shower"), Showers},
        {QStringLiteral("light rain showers"), Showers},
        {QStringLiteral("light showers"), Showers},
        {QStringLiteral("light rain"), Showers},
        {QStringLiteral("heavy rain"), Rain},
        {QStringLiteral("heavy showers"), Rain},
        {QStringLiteral("heavy shower"), Rain},
        {QStringLiteral("heavy rain shower"), Rain},
        {QStringLiteral("heavy rain showers"), Rain},
        {QStringLiteral("thundery shower"), Thunderstorm},
        {QStringLiteral("thundery showers"), Thunderstorm},
        {QStringLiteral("thunderstorm"), Thunderstorm},
        {QStringLiteral("cloudy with sleet"), RainSnow},
        {QStringLiteral("sleet shower"), RainSnow},
        {QStringLiteral("sleet showers"), RainSnow},
        {QStringLiteral("sleet"), RainSnow},
        {QStringLiteral("cloudy with hail"), Hail},
        {QStringLiteral("hail shower"), Hail},
        {QStringLiteral("hail showers"), Hail},
        {QStringLiteral("hail"), Hail},
        {QStringLiteral("light snow"), LightSnow},
        {QStringLiteral("light snow shower"), Flurries},
        {QStringLiteral("light snow showers"), Flurries},
        {QStringLiteral("cloudy with light snow"), LightSnow},
        {QStringLiteral("heavy snow"), Snow},
        {QStringLiteral("heavy snow shower"), Snow},
        {QStringLiteral("heavy snow showers"), Snow},
        {QStringLiteral("cloudy with heavy snow"), Snow},
        {QStringLiteral("na"), NotAvailable},
    };
}

QMap<QString, Ion::ConditionIcons> UKMETIon::setupNightIconMappings() const
{
    return QMap<QString, ConditionIcons>{
        {QStringLiteral("clear"), ClearNight},
        {QStringLiteral("clear sky"), ClearNight},
        {QStringLiteral("clear intervals"), PartlyCloudyNight},
        {QStringLiteral("sunny intervals"), PartlyCloudyDay}, // it's not really sunny
        {QStringLiteral("sunny"), ClearDay},
        {QStringLiteral("light cloud"), PartlyCloudyNight},
        {QStringLiteral("partly cloudy"), PartlyCloudyNight},
        {QStringLiteral("cloudy"), PartlyCloudyNight},
        {QStringLiteral("white cloud"), PartlyCloudyNight},
        {QStringLiteral("grey cloud"), Overcast},
        {QStringLiteral("thick cloud"), Overcast},
        {QStringLiteral("drizzle"), LightRain},
        {QStringLiteral("misty"), Mist},
        {QStringLiteral("mist"), Mist},
        {QStringLiteral("fog"), Mist},
        {QStringLiteral("foggy"), Mist},
        {QStringLiteral("tropical storm"), Thunderstorm},
        {QStringLiteral("hazy"), Mist},
        {QStringLiteral("light shower"), Showers},
        {QStringLiteral("light rain shower"), Showers},
        {QStringLiteral("light rain showers"), Showers},
        {QStringLiteral("light showers"), Showers},
        {QStringLiteral("light rain"), Showers},
        {QStringLiteral("heavy rain"), Rain},
        {QStringLiteral("heavy showers"), Rain},
        {QStringLiteral("heavy shower"), Rain},
        {QStringLiteral("heavy rain shower"), Rain},
        {QStringLiteral("heavy rain showers"), Rain},
        {QStringLiteral("thundery shower"), Thunderstorm},
        {QStringLiteral("thundery showers"), Thunderstorm},
        {QStringLiteral("thunderstorm"), Thunderstorm},
        {QStringLiteral("cloudy with sleet"), RainSnow},
        {QStringLiteral("sleet shower"), RainSnow},
        {QStringLiteral("sleet showers"), RainSnow},
        {QStringLiteral("sleet"), RainSnow},
        {QStringLiteral("cloudy with hail"), Hail},
        {QStringLiteral("hail shower"), Hail},
        {QStringLiteral("hail showers"), Hail},
        {QStringLiteral("hail"), Hail},
        {QStringLiteral("light snow"), LightSnow},
        {QStringLiteral("light snow shower"), Flurries},
        {QStringLiteral("light snow showers"), Flurries},
        {QStringLiteral("cloudy with light snow"), LightSnow},
        {QStringLiteral("heavy snow"), Snow},
        {QStringLiteral("heavy snow shower"), Snow},
        {QStringLiteral("heavy snow showers"), Snow},
        {QStringLiteral("cloudy with heavy snow"), Snow},
        {QStringLiteral("na"), NotAvailable},
    };
}

QMap<QString, Ion::WindDirections> UKMETIon::setupWindIconMappings() const
{
    return QMap<QString, WindDirections>{
        {QStringLiteral("northerly"), N},
        {QStringLiteral("north north easterly"), NNE},
        {QStringLiteral("north easterly"), NE},
        {QStringLiteral("east north easterly"), ENE},
        {QStringLiteral("easterly"), E},
        {QStringLiteral("east south easterly"), ESE},
        {QStringLiteral("south easterly"), SE},
        {QStringLiteral("south south easterly"), SSE},
        {QStringLiteral("southerly"), S},
        {QStringLiteral("south south westerly"), SSW},
        {QStringLiteral("south westerly"), SW},
        {QStringLiteral("west south westerly"), WSW},
        {QStringLiteral("westerly"), W},
        {QStringLiteral("west north westerly"), WNW},
        {QStringLiteral("north westerly"), NW},
        {QStringLiteral("north north westerly"), NNW},
        {QStringLiteral("calm"), VR},
    };
}

QMap<QString, Ion::ConditionIcons> const &UKMETIon::dayIcons() const
{
    static QMap<QString, ConditionIcons> const dval = setupDayIconMappings();
    return dval;
}

QMap<QString, Ion::ConditionIcons> const &UKMETIon::nightIcons() const
{
    static QMap<QString, ConditionIcons> const nval = setupNightIconMappings();
    return nval;
}

QMap<QString, Ion::WindDirections> const &UKMETIon::windIcons() const
{
    static QMap<QString, WindDirections> const wval = setupWindIconMappings();
    return wval;
}

QUrl UKMETIon::providerURL() const
{
    return QUrl(QStringLiteral("https://open.live.bbc.co.uk"));
}

void UKMETIon::findPlaces(std::shared_ptr<QPromise<std::shared_ptr<Locations>>> promise, const QString &searchString)
{
    m_locationPromise = promise;

    m_locationPromise->start();

    if (m_locationPromise->isCanceled()) {
        m_locationPromise->finish();
        clearLocationData();
        return;
    }

    m_searchString = searchString;

    // the API needs auto=true for partial-text searching
    // but unlike the normal query, using auto=true doesn't show locations which match the text but with different unicode
    // for example "hyderabad" with no auto matches "Hyderabad" and "Hyderābād"
    // but with auto matches only "Hyderabad"
    // so we merge the two results
    m_pendingSearchCount = 2;

    m_locationsData.clear();
    m_locationsAutoData.clear();

    const QUrl url(u"https://open.live.bbc.co.uk/locator/locations?s=%1&format=json"_s.arg(m_searchString));
    const auto getJob = requestAPIJob(url, m_locationsData);
    connect(getJob, &KJob::result, this, &UKMETIon::places_slotJobFinished);

    const QUrl autoUrl(u"https://open.live.bbc.co.uk/locator/locations?s=%1&format=json&auto=true"_s.arg(m_searchString));
    const auto getAutoJob = requestAPIJob(autoUrl, m_locationsAutoData);
    connect(getAutoJob, &KJob::result, this, &UKMETIon::places_slotJobAutoFinished);
}

void UKMETIon::fetchForecast(std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> promise, const QString &placeInfo)
{
    m_forecastPromise = promise;

    m_forecastPromise->start();

    if (m_forecastPromise->isCanceled()) {
        m_forecastPromise->finish();
        return;
    }

    qCDebug(WEATHER::ION::BBCUKMET) << "Update request for placeInfo: " << placeInfo;

    // place_name|id - Triggers receiving weather of place
    const QList<QString> info = placeInfo.split('|'_L1);

    if (info.size() == 4 && info[1] == "weather"_L1 && !m_isLegacy) {
        m_isLegacy = true;
        fetchForecast(m_forecastPromise, info[2] + '|'_L1 + info[3]);
        return;
    }

    if (info.count() != 2) {
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    m_weatherData = std::make_shared<WeatherData>();

    m_placeName = info.first();
    m_placeId = info.last();

    getObservation();
}

void UKMETIon::clearLocationData()
{
    m_pendingSearchCount = 0;
    m_locationPromise.reset();
    m_locationsData.clear();
    m_locationsAutoData.clear();
    m_locations.clear();
}

void UKMETIon::clearForecastData()
{
    m_isLegacy = false;
    m_forecastPromise.reset();
    m_weatherData.reset();
    m_forecastData.clear();
}

KJob *UKMETIon::requestAPIJob(const QUrl &url, QByteArray &result)
{
    KIO::TransferJob *getJob = KIO::get(url, KIO::Reload, KIO::HideProgressInfo);
    getJob->addMetaData(u"cookies"_s, u"none"_s);

    qCDebug(WEATHER::ION::BBCUKMET) << "Requesting URL:" << url;

    connect(getJob, &KIO::TransferJob::data, this, [&result](KIO::Job *job, const QByteArray &data) {
        Q_UNUSED(job);
        if (data.isEmpty()) {
            return;
        }
        result.append(data);
    });

    return getJob;
}

int UKMETIon::secondsToRetry()
{
    constexpr int MAX_RETRY_ATTEMPS = 5;

    m_retryAttempts++;

    if (m_retryAttempts > MAX_RETRY_ATTEMPS) {
        qCWarning(WEATHER::ION::BBCUKMET) << "Couldn't get a valid response after" << MAX_RETRY_ATTEMPS << "attempts";
        return -1;
    }

    const int delay_sec = 2 << m_retryAttempts; // exponential increase, starting at 4 seconds
    qCDebug(WEATHER::ION::BBCUKMET) << "Retry in" << delay_sec << "seconds";

    return delay_sec;
}

void UKMETIon::getObservation()
{
    if (m_forecastPromise->isCanceled()) {
        m_forecastPromise->finish();
        clearForecastData();
        return;
    }

    const QUrl url(u"https://weather-broker-cdn.api.bbci.co.uk/en/observation/%1"_s.arg(m_placeId));
    const auto getJob = requestAPIJob(url, m_forecastData);
    connect(getJob, &KJob::result, this, &UKMETIon::observation_slotJobFinished);
}

void UKMETIon::getForecast()
{
    if (m_forecastPromise->isCanceled()) {
        m_forecastPromise->finish();
        clearForecastData();
        return;
    }

    const QUrl url(u"https://weather-broker-cdn.api.bbci.co.uk/en/forecast/aggregated/%1"_s.arg(m_placeId));

    const auto getJob = requestAPIJob(url, m_forecastData);
    connect(getJob, &KJob::result, this, &UKMETIon::forecast_slotJobFinished);
}

void UKMETIon::readSearchData(const QByteArray &json)
{
    QJsonObject jsonDocumentObject = QJsonDocument::fromJson(json).object().value(u"response"_s).toObject();

    if (jsonDocumentObject.isEmpty()) {
        return;
    }
    QJsonValue resultsVariant = jsonDocumentObject.value(u"locations"_s);

    if (resultsVariant.isUndefined()) {
        // this is a response from an auto=true query
        resultsVariant = jsonDocumentObject.value(u"results"_s).toObject().value(u"results"_s);
    }

    const QJsonArray results = resultsVariant.toArray();

    for (const QJsonValue &resultValue : results) {
        QJsonObject result = resultValue.toObject();
        const QString id = result.value(u"id"_s).toString();
        const QString name = result.value(u"name"_s).toString();
        const QString area = result.value(u"container"_s).toString();
        const QString country = result.value(u"country"_s).toString();
        const QString placeType = result.value(u"placeType"_s).toString();

        if (id.isEmpty() || name.isEmpty() || area.isEmpty() || country.isEmpty()) {
            continue;
        }

        if (placeType == "region"_L1) {
            qCDebug(WEATHER::ION::BBCUKMET) << "Filtering out region:" << name << id;
            continue;
        }

        const QString fullName = u"%1, %2, %3"_s.arg(name, area, country);

        // Duplicate places can exist, show them too, but not if they have
        // the exact same id, which can happen since we're merging two results
        QString sourceKey = fullName;
        int duplicate = 1;
        while (m_locations.contains(sourceKey) && m_place.value(sourceKey).placeCode != id) {
            duplicate++;
            sourceKey = u"%1 (#%2)"_s.arg(fullName).arg(duplicate);
        }
        if (m_locations.contains(sourceKey)) {
            continue;
        }

        PlaceInfo &place = m_place[sourceKey];
        place.placeCode = id;
        place.station = name;
        place.displayName = fullName;

        m_locations.append(sourceKey);
    }

    qCDebug(WEATHER::ION::BBCUKMET) << "Search results:" << results.count() << "Total unique locations:" << m_locations.count()
                                    << "Pending calls:" << m_pendingSearchCount;
}

void UKMETIon::places_slotJobFinished(KJob *job)
{
    --m_pendingSearchCount;

    if (m_locationPromise->isCanceled()) {
        if (!m_pendingSearchCount) {
            m_locationPromise->finish();
            clearLocationData();
        }
        return;
    }

    // If Redirected, don't go to this routine
    if (!job->error()) {
        readSearchData(m_locationsData);
    }

    // Wait until the last search completes before serving the results
    if (m_pendingSearchCount == 0) {
        validate();
    }
}

void UKMETIon::places_slotJobAutoFinished(KJob *job)
{
    if (m_locationPromise->isCanceled()) {
        if (!m_pendingSearchCount) {
            m_locationPromise->finish();
            clearLocationData();
        }
        return;
    }

    --m_pendingSearchCount;

    // If Redirected, don't go to this routine
    if (!job->error()) {
        readSearchData(m_locationsAutoData);
    }

    // Wait until the last search completes before serving the results
    if (m_pendingSearchCount == 0) {
        validate();
    }
}

void UKMETIon::observation_slotJobFinished(KJob *job)
{
    Q_UNUSED(job);

    if (m_forecastPromise->isCanceled()) {
        m_forecastPromise->finish();
        clearForecastData();
        return;
    }

    QJsonParseError jsonError;
    const auto doc = QJsonDocument::fromJson(m_forecastData, &jsonError);

    if (doc.isNull()) {
        qCWarning(WEATHER::ION::BBCUKMET) << "Received invalid data:" << jsonError.errorString();
    } else if (const auto response = doc[u"response"_s].toObject(); !response.isEmpty()) {
        // Server returns some HTTP states as JSON data.
        const int errorCode = response[u"code"_s].toInt();
        qCWarning(WEATHER::ION::BBCUKMET) << "Received server error:" << errorCode << response[u"message"_s].toString();
        if (errorCode == 202) {
            // State "202 Accepted" means it's getting the data ready. Retry
            if (const int delay_sec = secondsToRetry(); delay_sec > 0) {
                QTimer::singleShot(delay_sec * 1000, [this]() {
                    getObservation();
                });
                return;
            }
        }
    } else {
        readObservationData(doc);
    }

    m_retryAttempts = 0;
    m_forecastData.clear();
    getForecast();
}

void UKMETIon::forecast_slotJobFinished(KJob *job)
{
    Q_UNUSED(job);

    if (m_forecastPromise->isCanceled()) {
        m_forecastPromise->finish();
        clearForecastData();
        return;
    }

    QJsonParseError jsonError;
    const auto doc = QJsonDocument::fromJson(m_forecastData, &jsonError);

    if (doc.isNull()) {
        qCWarning(WEATHER::ION::BBCUKMET) << "Received invalid data:" << jsonError.errorString();
    } else if (const auto response = doc[u"response"_s].toObject(); !response.isEmpty()) {
        // Server returns some HTTP states as JSON data.
        const int errorCode = response[u"code"_s].toInt();
        qCWarning(WEATHER::ION::BBCUKMET) << "Received server error:" << errorCode << response[u"message"_s].toString();
        if (errorCode == 202) {
            // State "202 Accepted" means it's getting the data ready. Retry
            if (const int delay_sec = secondsToRetry(); delay_sec > 0) {
                QTimer::singleShot(delay_sec * 1000, [this]() {
                    getForecast();
                });
                return;
            }
        }
    } else {
        readForecast(doc);
    }

    m_retryAttempts = 0;
    updateWeather();

    m_forecastPromise->finish();
    clearForecastData();
}

bool UKMETIon::readObservationData(const QJsonDocument &doc)
{
    WeatherData &data = *m_weatherData;
    WeatherData::Observation &current = data.current;

    // Station data
    const QJsonObject station = doc[u"station"_s].toObject();
    if (!station.isEmpty()) {
        data.stationName = station[u"name"_s].toString();
        data.stationLatitude = station[u"latitude"_s].toDouble(qQNaN());
        data.stationLongitude = station[u"longitude"_s].toDouble(qQNaN());
    }

    // Observation data
    const QJsonArray observations = doc[u"observations"_s].toArray();
    if (observations.isEmpty()) {
        qCDebug(WEATHER::ION::BBCUKMET) << "Malformed observation report" << doc;
        return false;
    }
    const QJsonObject observation = observations.first().toObject();

    current = WeatherData::Observation(); // Clean-up

    current.observationDateTime = QDateTime::fromString(observation[u"updateTimestamp"_s].toString(), Qt::ISODate);
    current.obsTime = observation[u"localDate"_s].toString() + u" "_s + observation[u"localTime"_s].toString();

    current.isNight = isNightTime(current.observationDateTime, data.stationLatitude, data.stationLongitude);

    current.condition = observation[u"weatherTypeText"_s].toString();
    if (current.condition == "null"_L1 || current.condition == "Not Available"_L1) {
        current.condition.clear();
    }

    current.temperature_C = observation[u"temperature"_s][u"C"_s].toDouble(qQNaN());
    current.humidity = observation[u"humidityPercent"_s].toDouble(qQNaN());
    current.pressure = observation[u"pressureMb"_s].toDouble(qQNaN());

    // <ion.h> "Pressure Tendency": string, "rising", "falling", "steady"
    current.pressureTendency = observation[u"pressureDirection"_s].toString().toLower();
    if (current.pressureTendency == "no change"_L1) {
        current.pressureTendency = u"steady"_s;
    }

    current.windSpeed_miles = observation[u"wind"_s][u"windSpeedMph"_s].toDouble(qQNaN());
    current.windDirection = (current.windSpeed_miles > 0) ? observation[u"wind"_s][u"windDirectionAbbreviation"_s].toString() : u"VR"_s;

    current.visibilityStr = observation[u"visibility"_s].toString();

    qCDebug(WEATHER::ION::BBCUKMET) << "Read observation data:" << m_weatherData->current.obsTime << m_weatherData->current.condition;

    return true;
}

bool UKMETIon::readForecast(const QJsonDocument &doc)
{
    const QJsonArray info = doc[u"forecasts"_s].toArray();
    if (info.isEmpty()) {
        qCDebug(WEATHER::ION::BBCUKMET) << "Malformed forecast report" << doc;
    }

    WeatherData &weatherData = *m_weatherData;
    weatherData.forecasts.clear();

    // Indicates if the forecast includes a first report for tonight
    bool isTonight = doc[u"isNight"_s].toBool(false);

    for (const QJsonValue &forecast : info) {
        const QJsonObject report = forecast[u"summary"_s][u"report"_s].toObject();
        if (report.isEmpty()) {
            continue;
        }

        weatherData.forecasts << parseForecastReport(report, isTonight);
        isTonight = false; // Just use it on the first report
    }

    qCDebug(WEATHER::ION::BBCUKMET) << "Read forecast data:" << m_weatherData->forecasts.count() << "days";

    return true;
}

WeatherData::ForecastInfo UKMETIon::parseForecastReport(const QJsonObject &report, bool isNight) const
{
    WeatherData::ForecastInfo forecast;

    forecast.period = QDate::fromString(report[u"localDate"_s].toString(), Qt::ISODate); // "YYYY-MM-DD" (ISO8601)
    forecast.isNight = isNight;

    forecast.summary = report[u"weatherTypeText"_s].toString().toLower();
    forecast.iconName = getWeatherIcon(isNight ? nightIcons() : dayIcons(), forecast.summary);

    forecast.tempLow = report[u"minTempC"_s].toDouble(qQNaN());
    if (!isNight) { // Don't include max temperatures in a nightly report
        forecast.tempHigh = report[u"maxTempC"_s].toDouble(qQNaN());
    }
    forecast.precipitationPct = report[u"precipitationProbabilityInPercent"_s].toInt();
    forecast.windSpeed = report[u"windSpeedKph"_s].toDouble(qQNaN());
    forecast.windDirection = report[u"windDirectionAbbreviation"_s].toString();

    return forecast;
}

void UKMETIon::validate()
{
    qCDebug(WEATHER::ION::BBCUKMET) << "validating...";
    if (m_locations.isEmpty()) {
        m_locationPromise->finish();
        m_locationPromise.reset();
        qCDebug(WEATHER::ION::BBCUKMET) << "locations are empty. Return";
        return;
    }

    auto locations = std::make_shared<Locations>();

    for (const QString &place : std::as_const(m_locations)) {
        Location location;
        location.setDisplayName(place);
        location.setStation(m_place[place].station);
        location.setCode(m_place[place].placeCode);
        location.setPlaceInfo(place + u"|"_s + m_place[place].placeCode);
        locations->addLocation(location);
    }

    qCDebug(WEATHER::ION::BBCUKMET) << "Validated. Clear and return";

    m_locationPromise->addResult(locations);
    m_locationPromise->finish();
    clearLocationData();
}

void UKMETIon::updateWeather()
{
    auto forecast = std::make_shared<Forecast>();

    Station station;

    station.setPlace(m_placeName);
    station.setStation(m_weatherData->stationName);
    const bool stationCoordsValid = !qIsNaN(m_weatherData->stationLatitude) && !qIsNaN(m_weatherData->stationLongitude);
    if (stationCoordsValid) {
        station.setCoordinates(m_weatherData->stationLatitude, m_weatherData->stationLongitude);
    }

    if (m_isLegacy) {
        station.setNewPlaceInfo(m_placeName + u"|"_s + m_placeId);
    }

    forecast->setStation(station);

    MetaData metaData;

    metaData.setCredit(i18nc("credit line, keep string short", "Data from BBC\302\240Weather"));
    metaData.setCreditURL(u"https://www.bbc.com/weather/%1"_s.arg(m_placeId));

    metaData.setHumidityUnit(KUnitConversion::Percent);
    metaData.setVisibilityUnit(KUnitConversion::NoUnit);
    metaData.setTemperatureUnit(KUnitConversion::Celsius);
    metaData.setPressureUnit(KUnitConversion::Millibar);
    metaData.setWindSpeedUnit(KUnitConversion::MilePerHour);

    forecast->setMetadata(metaData);

    LastObservation lastObservation;

    const WeatherData::Observation &current = m_weatherData->current;
    if (current.observationDateTime.isValid()) {
        lastObservation.setObservationTimestamp(current.observationDateTime);
    }

    if (!current.condition.isEmpty()) {
        lastObservation.setCurrentConditions(i18nc("weather condition", current.condition.toUtf8().data()));
        lastObservation.setConditionIcon(getWeatherIcon(current.isNight ? nightIcons() : dayIcons(), current.condition));
    }

    if (!qIsNaN(current.humidity)) {
        lastObservation.setHumidity(current.humidity);
    }

    if (!current.visibilityStr.isEmpty()) {
        lastObservation.setVisibility(i18nc("visibility", current.visibilityStr.toUtf8().data()));
    }

    if (!qIsNaN(current.temperature_C)) {
        lastObservation.setTemperature(current.temperature_C);
    }

    if (!qIsNaN(current.pressure)) {
        lastObservation.setPressure(current.pressure);
        if (!current.pressureTendency.isEmpty()) {
            if (current.pressureTendency == "rising"_L1) {
                lastObservation.setPressureTendency(i18n("Rising"));
            } else if (current.pressureTendency == "falling"_L1) {
                lastObservation.setPressureTendency(i18n("Falling"));
            } else {
                lastObservation.setPressureTendency(i18n("Steady"));
            }
        }
    }

    if (!qIsNaN(current.windSpeed_miles)) {
        lastObservation.setWindSpeed(current.windSpeed_miles);
        lastObservation.setWindDirection(current.windDirection);
    }

    forecast->setLastObservation(lastObservation);

    // Daily forecast info

    auto futureDays = std::make_shared<FutureDays>();

    int day = 0;
    for (const WeatherData::ForecastInfo &forecast : std::as_const(m_weatherData->forecasts)) {
        FutureDayForecast futureDayForecast;
        QString weekDayLabel = QLocale().toString(forecast.period, u"ddd"_s);
        if (day == 0 && forecast.period <= QDate::currentDate()) {
            weekDayLabel = forecast.isNight ? i18nc("Short for Tonight", "Tonight") : i18nc("Short for Today", "Today");
        }

        futureDayForecast.setWeekDay(weekDayLabel);

        FutureForecast futureForecast;
        futureForecast.setConditionIcon(forecast.iconName);
        futureForecast.setCondition(i18nc("weather forecast", forecast.summary.toUtf8().data()));
        if (!qIsNaN(forecast.tempHigh)) {
            futureForecast.setHighTemp(forecast.tempHigh);
        }
        if (!qIsNaN(forecast.tempLow)) {
            futureForecast.setLowTemp(forecast.tempLow);
        }
        futureForecast.setConditionProbability(forecast.precipitationPct);
        ++day;

        futureDayForecast.setDaytime(futureForecast);

        futureDays->addDay(futureDayForecast);
        // Limit the forecast days to 7 days (the applet is not ready for more)
        if (day >= 7) {
            break;
        }
    }

    forecast->setFutureDays(futureDays);

    m_forecastPromise->addResult(forecast);

    qCDebug(WEATHER::ION::BBCUKMET) << "Updated weather data for" << m_placeName;
}

#include "ion_bbcukmet.moc"
#include "moc_ion_bbcukmet.cpp"
