/*
    SPDX-FileCopyrightText: 2007-2009, 2019 Shawn Starr <shawn.starr@rogers.com>
    SPDX-FileCopyrightText: 2024 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/* Ion for NOAA's National Weather Service openAPI data */

#include "ion_noaa.h"

#include "noaa_debug.h"

#include <KIO/TransferJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocale>
#include <QStandardPaths>
#include <QTimeZone>

K_PLUGIN_CLASS_WITH_JSON(NOAAIon, "metadata.json")

using namespace Qt::StringLiterals;
using namespace KUnitConversion;

QMap<QString, Ion::ConditionIcons> NOAAIon::setupConditionIconMappings() const
{
    QMap<QString, ConditionIcons> conditionList;
    return conditionList;
}

QMap<QString, Ion::ConditionIcons> const &NOAAIon::conditionIcons() const
{
    static QMap<QString, ConditionIcons> const condval = setupConditionIconMappings();
    return condval;
}

// ctor, dtor
NOAAIon::NOAAIon(QObject *parent, const QVariantList &args)
    : Ion(parent)
{
    Q_UNUSED(args);

    qCDebug(WEATHER::ION::NOAA) << "Start initializing";

    // Schedule the API calls according to the previous information required
    connect(this, &NOAAIon::observationUpdated, this, &NOAAIon::getForecast);
    connect(this, &NOAAIon::observationUpdated, this, &NOAAIon::getAlerts);

    // Get the list of stations for search, location and observation data
    getStationList();
    qCDebug(WEATHER::ION::NOAA) << "Initialization complete";
}

NOAAIon::~NOAAIon()
{
    qCDebug(WEATHER::ION::NOAA) << "Deleting";
}

void NOAAIon::findPlaces(std::shared_ptr<QPromise<std::shared_ptr<Locations>>> promise, const QString &searchString)
{
    m_locationPromise = promise;

    if (m_locationPromise->isCanceled()) {
        qCDebug(WEATHER::ION::NOAA) << "Locations search cancelled. Return";
        m_locationPromise->finish();
        m_locationPromise.reset();
        return;
    }

    if (m_places.isEmpty()) {
        qCWarning(WEATHER::ION::NOAA) << "Places is empty when fetching locations. Return";
        m_locationPromise->finish();
        m_locationPromise.reset();
        return;
    }

    QString sourceNormalized = searchString.toUpper();

    // If the source name might look like a station ID, check these too and return the name
    bool checkState = searchString.size() == 2;

    auto locations = std::make_shared<Locations>();
    locations->setCredit(u"NOAA National Weather Service"_s);

    for (auto it = m_places.constBegin(); it != m_places.constEnd(); ++it) {
        if (checkState) {
            if (it.value().stateName == searchString) {
                Location location;
                location.setDisplayName(it.key());
                location.setStation(it.value().stateName);
                location.setPlaceInfo(it.key());
                location.setCode(it.value().stationID);
                locations->addLocation(location);
            }
        } else if (it.key().toUpper().contains(sourceNormalized)) {
            Location location;
            location.setDisplayName(it.key());
            location.setStation(it.value().stateName);
            location.setPlaceInfo(it.key());
            location.setCode(it.value().stationID);
            locations->addLocation(location);
        } else if (it.value().stationID == sourceNormalized) {
            Location location;
            location.setDisplayName(it.key());
            location.setStation(it.value().stateName);
            location.setPlaceInfo(it.key());
            location.setCode(it.value().stationID);
            locations->addLocation(location);
        }
    }

    qCDebug(WEATHER::ION::NOAA) << "Number of locations added: " << locations->rowCount();

    m_locationPromise->addResult(locations);
    m_locationPromise->finish();
    m_locationPromise.reset();
}

KJob *NOAAIon::requestAPIJob(const QUrl &url, Callback onResult)
{
    KIO::TransferJob *getJob = KIO::get(url, KIO::Reload, KIO::HideProgressInfo);

    qCDebug(WEATHER::ION::NOAA) << "Requesting URL:" << url;

    auto receivedData = std::make_shared<QByteArray>();

    connect(getJob, &KIO::TransferJob::data, this, [this, receivedData](KIO::Job *job, const QByteArray &data) {
        Q_UNUSED(job)

        if (data.isEmpty()) {
            return;
        }
        receivedData->append(data);
    });

    if (!onResult) {
        return getJob;
    }

    connect(getJob, &KJob::result, this, [this, onResult, receivedData](KJob *job) {
        // Invoke callback method
        (this->*onResult)(job, *receivedData);
    });

    return getJob;
}

// Opens and reads the list of weather stations, which provide a place name,
// the station ID code and the coordinates of the station
void NOAAIon::getStationList()
{
    const QString stationsFileName = u"plasma/weather/noaa_station_list.xml"_s;
    const QString stationsPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, stationsFileName);

    if (stationsPath.isEmpty()) {
        qCWarning(WEATHER::ION::NOAA) << "Couldn't find file" << stationsFileName << "on the local data path";
        return;
    }

    QFile stationsFile(stationsPath);
    if (!stationsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(WEATHER::ION::NOAA) << "Couldn't open stations file:" << stationsPath << stationsFile.errorString();
        return;
    }

    QXmlStreamReader reader = QXmlStreamReader(&stationsFile);
    readStationList(reader);
    stationsFile.close();

    qCDebug(WEATHER::ION::NOAA) << "Total stations: " << m_places.count();
}

void NOAAIon::fetchForecast(std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> promise, const QString &placeInfo)
{
    m_forecastPromise = promise;

    if (m_forecastPromise->isCanceled()) {
        qCDebug(WEATHER::ION::NOAA) << "Forecast fetching cancelled. Return";
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    if (placeInfo.isEmpty()) {
        qCDebug(WEATHER::ION::NOAA) << "PlaceInfo is empty. Return";
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    if (m_places.isEmpty()) {
        qCWarning(WEATHER::ION::NOAA) << "Places is empty when fetching forecast. Return";
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    // If this is empty we have no valid data, send out an error and abort.
    if (!m_places.contains(placeInfo)) {
        qCDebug(WEATHER::ION::NOAA) << "Places not found when fetching forecast. Return";
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    const StationInfo &station = m_places.value(placeInfo);

    m_weatherData = std::make_shared<WeatherData>();
    WeatherData &data = *m_weatherData;

    data.locationName = station.stationName;
    data.stationID = station.stationID;
    data.stationLongitude = station.location.x();
    data.stationLatitude = station.location.y();

    qCDebug(WEATHER::ION::NOAA) << "Established station:" << data.locationName << data.stationID << data.stationLatitude << data.stationLongitude;

    getObservation();
    getPointsInfo();
}

// handle when no XML tag is found
void NOAAIon::parseUnknownElement(QXmlStreamReader &xml) const
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            parseUnknownElement(xml);
        }
    }
}

void NOAAIon::parseStationID(QXmlStreamReader &xml)
{
    QString state;
    QString stationName;
    QString stationID;
    float latitude = qQNaN();
    float longitude = qQNaN();

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == "station"_L1) {
            if (!stationID.isEmpty()) {
                StationInfo info;
                info.stateName = state;
                info.stationName = stationName;
                info.stationID = stationID;
                info.location = QPointF(longitude, latitude);

                QString key = "%1, %2"_L1.arg(stationName, state);
                m_places[key] = info;
            }
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == "station_id"_L1) {
                stationID = xml.readElementText();
            } else if (elementName == "state"_L1) {
                state = xml.readElementText();
            } else if (elementName == "station_name"_L1) {
                stationName = xml.readElementText();
            } else if (elementName == "latitude"_L1) {
                latitude = xml.readElementText().toFloat();
            } else if (elementName == "longitude"_L1) {
                longitude = xml.readElementText().toFloat();
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

// Parse the city list and store into a QMap
void NOAAIon::readStationList(QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            // start element
            if (xml.name() == QLatin1String("wx_station_index")) {
                continue;
            }

            if (xml.name() == "station"_L1) {
                parseStationID(xml);
            } else {
                parseUnknownElement(xml);
            }
        }
    }
    if (xml.error()) {
        qCDebug(WEATHER::ION::NOAA) << "Error reading station list. Return";
    } else {
        qCDebug(WEATHER::ION::NOAA) << "Station list read. Total locations: " << m_places.size();
    }
}

void NOAAIon::getObservation()
{
    const QString &stationID = m_weatherData->stationID;
    auto job = requestAPIJob(QUrl(u"https://api.weather.gov/stations/%1/observations/latest"_s.arg(stationID)), &NOAAIon::readObservation);
    m_jobs.insert(job);
}

void NOAAIon::readObservation(KJob *job, const QByteArray &data)
{
    m_jobs.remove(job);

    // If the points info job ended with error then stop processing to avoid malformed forecast
    if (m_weatherData->isPointsInfoDataError) {
        qCDebug(WEATHER::ION::NOAA) << "Error reading points info data. Stop reading observation data";
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        m_weatherData.reset();
        return;
    }

    if (m_forecastPromise->isCanceled()) {
        qCDebug(WEATHER::ION::NOAA) << "Forecast fetching cancelled. Return";
        if (m_jobs.isEmpty()) {
            // if cancelled then don't remove unneeded data until last job is finished because otherwise it
            // can lead to not correct forecast data or even crash.
            m_forecastPromise->finish();
            m_forecastPromise.reset();
            m_weatherData.reset();
        }
        return;
    }

    if (job->error()) {
        qCWarning(WEATHER::ION::NOAA) << "Error retrieving data" << job->errorText();
        if (m_jobs.isEmpty()) {
            m_forecastPromise->finish();
            m_forecastPromise.reset();
            m_weatherData.reset();
        } else {
            m_weatherData->isObservationDataError = true;
        }
        return;
    }

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);

    if (doc.isNull()) {
        qCWarning(WEATHER::ION::NOAA) << "Received invalid JSON data:" << jsonError.errorString();
        if (m_jobs.isEmpty()) {
            m_forecastPromise->finish();
            m_forecastPromise.reset();
            m_weatherData.reset();
        } else {
            m_weatherData->isObservationDataError = true;
        }
        return;
    }

    const QJsonValue properties = doc[u"properties"_s];
    if (!properties.isObject()) {
        qCWarning(WEATHER::ION::NOAA) << "Malformed observation report" << doc;
        if (m_jobs.isEmpty()) {
            m_forecastPromise->finish();
            m_forecastPromise.reset();
            m_weatherData.reset();
        } else {
            m_weatherData->isObservationDataError = true;
        }
        return;
    }

    WeatherData::Observation &observation = m_weatherData->observation;
    observation = WeatherData::Observation{};

    observation.weather = properties[u"textDescription"_s].toString();
    observation.timestamp = QDateTime::fromString(properties[u"timestamp"_s].toString(), Qt::ISODate);
    observation.isNight = isNightTime(observation.timestamp, m_weatherData->stationLatitude, m_weatherData->stationLongitude);

    observation.temperature_F = parseQV(properties[u"temperature"_s], Fahrenheit);
    observation.humidity = parseQV(properties[u"relativeHumidity"_s], Percent);
    observation.pressure = parseQV(properties[u"barometricPressure"_s], InchesOfMercury);
    observation.visibility = parseQV(properties[u"visibility"_s], Mile);

    observation.windDirection = parseQV(properties[u"windDirection"_s], Degree);
    observation.windSpeed = parseQV(properties[u"windSpeed"_s], MilePerHour);
    observation.windGust = parseQV(properties[u"windGust"_s], MilePerHour);

    observation.dewpoint_F = parseQV(properties[u"dewpoint"_s], Fahrenheit);
    observation.heatindex_F = parseQV(properties[u"heatIndex"_s], Fahrenheit);
    observation.windchill_F = parseQV(properties[u"windChill"_s], Fahrenheit);

    qCDebug(WEATHER::ION::NOAA) << "Received observation data:" << observation.timestamp << observation.weather;

    if (m_jobs.isEmpty()) {
        Q_EMIT observationUpdated();
    }
}

void NOAAIon::updateWeather()
{
    const WeatherData::Observation &observation = m_weatherData->observation;

    auto returnForecast = std::make_shared<Forecast>();

    Station station;
    station.setPlace(m_weatherData->locationName);
    station.setStation(m_weatherData->stationID);
    if (!qIsNaN(m_weatherData->stationLatitude) && !qIsNaN(m_weatherData->stationLongitude)) {
        station.setCoordinates(m_weatherData->stationLatitude, m_weatherData->stationLongitude);
    }

    returnForecast->setStation(station);

    MetaData metaData;
    metaData.setCredit(i18nc("credit line, keep string short)", "Data from NOAA National\302\240Weather\302\240Service"));
    metaData.setTemperatureUnit(Fahrenheit);
    metaData.setPressureUnit(InchesOfMercury);
    metaData.setVisibilityUnit(Mile);
    metaData.setHumidityUnit(Percent);
    metaData.setWindSpeedUnit(MilePerHour);

    returnForecast->setMetadata(metaData);

    // Real weather - Current conditions
    LastObservation lastObservation;
    if (observation.timestamp.isValid()) {
        lastObservation.setObservationTimestamp(observation.timestamp);
    }

    const QString conditionI18n = observation.weather.isEmpty() ? i18n("N/A") : i18nc("weather condition", observation.weather.toUtf8().data());

    lastObservation.setCurrentConditions(conditionI18n);
    qCDebug(WEATHER::ION::NOAA) << "i18n condition string: " << qPrintable(conditionI18n);

    const QString weather = observation.weather.toLower();
    ConditionIcons condition = getConditionIcon(weather, !observation.isNight);
    lastObservation.setConditionIcon(getWeatherIcon(condition));

    if (!qIsNaN(observation.temperature_F)) {
        lastObservation.setTemperature(observation.temperature_F);
    }

    if (!qIsNaN(observation.windchill_F)) {
        lastObservation.setWindchill(observation.windchill_F);
    }

    if (!qIsNaN(observation.heatindex_F)) {
        lastObservation.setHeatIndex(observation.heatindex_F);
    }

    if (!qIsNaN(observation.dewpoint_F)) {
        lastObservation.setDewpoint(observation.dewpoint_F);
    }

    if (!qIsNaN(observation.pressure)) {
        lastObservation.setPressure(observation.pressure);
    }

    if (!qIsNaN(observation.visibility)) {
        lastObservation.setVisibility(observation.visibility);
    }

    if (!qIsNaN(observation.humidity)) {
        lastObservation.setHumidity(observation.humidity);
    }

    if (!qIsNaN(observation.windSpeed)) {
        lastObservation.setWindSpeed(observation.windSpeed);
    }

    if (!qIsNaN(observation.windGust)) {
        lastObservation.setWindGust(observation.windGust);
    }

    if (!qIsNaN(observation.windSpeed) && qFuzzyIsNull(observation.windSpeed)) {
        lastObservation.setWindDirection(u"VR"_s); // Variable/calm
    } else if (!qIsNaN(observation.windDirection)) {
        lastObservation.setWindDirection(windDirectionFromAngle(observation.windDirection));
    }

    returnForecast->setLastObservation(lastObservation);

    auto futureDays = std::make_shared<FutureDays>();

    // Daily forecasts
    const auto &forecasts = m_weatherData->forecasts;

    // forecasts is a list with mixed day and night forecasts. Convert them to
    // FutureDays which have day and night spit.
    int dayPosition = 0;
    while (dayPosition < forecasts.size()) {
        const auto &current = forecasts[dayPosition];

        // Sometimes the forecast for the later days is unavailable, so skip
        // remaining days since their forecast data is probably unavailable.
        if (current.summary.isEmpty()) {
            break;
        }

        FutureDayForecast futureDayForecast;
        futureDayForecast.setMonthDay(current.day);

        if (current.isDayTime) {
            // Valid daytime
            ConditionIcons icon = getConditionIcon(current.summary.toLower(), current.isDayTime);
            QString iconName = getWeatherIcon(icon);
            FutureForecast futureDaytimeForecast;
            futureDaytimeForecast.setConditionIcon(iconName);
            futureDaytimeForecast.setCondition(i18nForecast(current.summary));
            if (!qIsNaN(current.high)) {
                futureDaytimeForecast.setHighTemp(current.high);
            }
            if (!qIsNaN(current.low)) {
                futureDaytimeForecast.setHighTemp(current.low);
            }
            futureDaytimeForecast.setConditionProbability(current.precipitation);

            futureDayForecast.setDaytime(futureDaytimeForecast);

            ++dayPosition;

            // Check the next forecast for a valid night with the same day
            if (dayPosition < forecasts.size()) {
                const auto &next = forecasts[dayPosition];

                // if the next forecast is an empty forecast then so skip
                // remaining days since their forecast data is probably unavailable.
                if (next.summary.isEmpty()) {
                    futureDays->addDay(futureDayForecast);
                    break;
                }

                // if we have day and night forecasts of the same month day then add it to the full day forecast
                if (!next.isDayTime && next.day == current.day) {
                    ConditionIcons icon = getConditionIcon(next.summary.toLower(), next.isDayTime);
                    QString iconName = getWeatherIcon(icon);
                    FutureForecast futureNightForecast;
                    futureNightForecast.setConditionIcon(iconName);
                    futureNightForecast.setCondition(i18nForecast(next.summary));
                    if (!qIsNaN(next.high)) {
                        futureNightForecast.setHighTemp(next.high);
                    }
                    if (!qIsNaN(next.low)) {
                        futureNightForecast.setHighTemp(next.low);
                    }
                    futureNightForecast.setConditionProbability(next.precipitation);
                    futureDayForecast.setNight(futureNightForecast);
                    ++dayPosition;
                }
            }
        } else {
            // Valid night forecast without a paired daytime
            ConditionIcons icon = getConditionIcon(current.summary.toLower(), current.isDayTime);
            QString iconName = getWeatherIcon(icon);
            FutureForecast futureNightForecast;
            futureNightForecast.setConditionIcon(iconName);
            futureNightForecast.setCondition(i18nForecast(current.summary));
            if (!qIsNaN(current.high)) {
                futureNightForecast.setHighTemp(current.high);
            }
            if (!qIsNaN(current.low)) {
                futureNightForecast.setHighTemp(current.low);
            }
            futureNightForecast.setConditionProbability(current.precipitation);

            futureDayForecast.setNight(futureNightForecast);
            ++dayPosition;
        }

        futureDays->addDay(futureDayForecast);
    }

    returnForecast->setFutureDays(futureDays);

    if (!m_weatherData->isAlertsDataError) {
        auto warnings = std::make_shared<Warnings>();

        for (const WeatherData::Alert &alert : m_weatherData->alerts) {
            // TODO: Add a Headline parameter to the plugin and the applet
            Warning warning(alert.priority, u"<p><b>%1</b></p>%2"_s.arg(alert.headline, alert.description));
            warning.setTimestamp(QLocale().toString(alert.startTime, QLocale::ShortFormat));
            warnings->addWarning(warning);
        }

        returnForecast->setWarnings(warnings);
    }

    qCDebug(WEATHER::ION::NOAA) << "Updated weather data for" << m_weatherData->locationName;

    m_forecastPromise->addResult(returnForecast);
    m_weatherData.reset();
    m_forecastPromise->finish();
    m_forecastPromise.reset();
}

/**
 * Determine the condition icon based on the list of possible NOAA weather conditions as defined at
 * <https://www.weather.gov/xml/current_obs/weather.php> and
 * <https://graphical.weather.gov/xml/mdl/XML/Design/MDL_XML_Design.htm#_Toc141760782>
 * Since the number of NOAA weather conditions need to be fitted into the narowly defined groups in IonInterface::ConditionIcons, we
 * try to group the NOAA conditions as best as we can based on their priorities/severity.
 * TODO: summaries "Hot" & "Cold" have no proper matching entry in ConditionIcons, consider extending it
 */
Ion::ConditionIcons NOAAIon::getConditionIcon(const QString &weather, bool isDayTime) const
{
    ConditionIcons result;
    // Consider any type of storm, tornado or funnel to be a thunderstorm.
    if (weather.contains("thunderstorm"_L1) || weather.contains("funnel"_L1) || weather.contains("tornado"_L1) || weather.contains("storm"_L1)
        || weather.contains("tstms"_L1)) {
        if (weather.contains("vicinity"_L1) || weather.contains("chance"_L1)) {
            result = isDayTime ? ChanceThunderstormDay : ChanceThunderstormNight;
        } else {
            result = Thunderstorm;
        }

    } else if (weather.contains("pellets"_L1) || weather.contains("crystals"_L1) || weather.contains("hail"_L1)) {
        result = Hail;

    } else if (((weather.contains("rain"_L1) || weather.contains("drizzle"_L1) || weather.contains("showers"_L1)) && weather.contains("snow"_L1))
               || weather.contains("wintry mix"_L1)) {
        result = RainSnow;

    } else if (weather.contains("flurries"_L1)) {
        result = Flurries;

    } else if (weather.contains("snow"_L1) && weather.contains("light"_L1)) {
        result = LightSnow;

    } else if (weather.contains("snow"_L1)) {
        if (weather.contains("vicinity"_L1) || weather.contains("chance"_L1)) {
            result = isDayTime ? ChanceSnowDay : ChanceSnowNight;
        } else {
            result = Snow;
        }

    } else if (weather.contains("freezing rain"_L1)) {
        result = FreezingRain;

    } else if (weather.contains("freezing drizzle"_L1)) {
        result = FreezingDrizzle;

    } else if (weather.contains("cold"_L1)) {
        // temperature condition has not hint about air ingredients, so let's assume chance of snow
        result = isDayTime ? ChanceSnowDay : ChanceSnowNight;

    } else if (weather.contains("showers"_L1)) {
        if (weather.contains("vicinity"_L1) || weather.contains("chance"_L1)) {
            result = isDayTime ? ChanceShowersDay : ChanceShowersNight;
        } else {
            result = Showers;
        }
    } else if (weather.contains("light rain"_L1) || weather.contains("drizzle"_L1)) {
        result = LightRain;

    } else if (weather.contains("rain"_L1)) {
        result = Rain;

    } else if (weather.contains("few clouds"_L1) || weather.contains("mostly sunny"_L1) || weather.contains("mostly clear"_L1)
               || weather.contains("increasing clouds"_L1) || weather.contains("becoming cloudy"_L1) || weather.contains("clearing"_L1)
               || weather.contains("decreasing clouds"_L1) || weather.contains("becoming sunny"_L1)) {
        if (weather.contains("breezy"_L1) || weather.contains("wind"_L1) || weather.contains("gust"_L1)) {
            result = isDayTime ? FewCloudsWindyDay : FewCloudsWindyNight;
        } else {
            result = isDayTime ? FewCloudsDay : FewCloudsNight;
        }

    } else if (weather.contains("partly cloudy"_L1) || weather.contains("partly sunny"_L1) || weather.contains("partly clear"_L1)) {
        if (weather.contains("breezy"_L1) || weather.contains("wind"_L1) || weather.contains("gust"_L1)) {
            result = isDayTime ? PartlyCloudyWindyDay : PartlyCloudyWindyNight;
        } else {
            result = isDayTime ? PartlyCloudyDay : PartlyCloudyNight;
        }

    } else if (weather.contains("overcast"_L1) || weather.contains("cloudy"_L1)) {
        if (weather.contains("breezy"_L1) || weather.contains("wind"_L1) || weather.contains("gust"_L1)) {
            result = OvercastWindy;
        } else {
            result = Overcast;
        }

    } else if (weather.contains("haze"_L1) || weather.contains("smoke"_L1) || weather.contains("dust"_L1) || weather.contains("sand"_L1)) {
        result = Haze;

    } else if (weather.contains("fair"_L1) || weather.contains("clear"_L1) || weather.contains("sunny"_L1)) {
        if (weather.contains("breezy"_L1) || weather.contains("wind"_L1) || weather.contains("gust"_L1)) {
            result = isDayTime ? ClearWindyDay : ClearWindyNight;
        } else {
            result = isDayTime ? ClearDay : ClearNight;
        }

    } else if (weather.contains("fog"_L1)) {
        result = Mist;

    } else if (weather.contains("hot"_L1)) {
        // temperature condition has not hint about air ingredients, so let's assume the sky is clear when it is hot
        if (weather.contains("breezy"_L1) || weather.contains("wind"_L1) || weather.contains("gust"_L1)) {
            result = isDayTime ? ClearWindyDay : ClearWindyNight;
        } else {
            result = isDayTime ? ClearDay : ClearNight;
        }

    } else if (weather.contains("breezy"_L1) || weather.contains("wind"_L1) || weather.contains("gust"_L1)) {
        // Assume a clear sky when it's windy but no clouds have been mentioned
        result = isDayTime ? ClearWindyDay : ClearWindyNight;
    } else {
        result = NotAvailable;
    }

    return result;
}

QString NOAAIon::i18nForecast(const QString &summary) const
{
    const QStringList conditions = summary.split(u" then "_s, Qt::SkipEmptyParts, Qt::CaseInsensitive);

    QStringList i18nSummary;
    for (const auto &condition : conditions) {
        i18nSummary << i18nc("weather forecast", condition.toUtf8().data());
    }

    // i18n: The forecast summary can include several single conditions that follow
    // a temporary sequence, separated by " then ". Also include spaces if necessary.
    // If there is no suitable separtor, a sentence separator might do.
    const QString separator = i18nc("Separator between forecast conditions that follow a temporal sequence (ex. \"Rain then Sunny\")", " then ");
    return i18nSummary.join(separator);
}

/* UnitOfMeasure
 * https://www.weather.gov/documentation/services-web-api
 * pattern: ^((wmo|uc|wmoUnit|nwsUnit):)?.*$
 * A string denoting a unit of measure, expressed in the format "{unit}" or "{namespace}:{unit}".
 * Units with the namespace "wmo" or "wmoUnit" are defined in the World Meteorological Organization Codes Registry at
 * http://codes.wmo.int/common/unit and should be canonically resolvable to http://codes.wmo.int/common/unit/{unit}.
 * Units with the namespace "nwsUnit" are currently custom and do not align to any standard.
 * Units with no namespace or the namespace "uc" are compliant with the Unified Code for Units of Measure
 * syntax defined at https://unitsofmeasure.org/. This also aligns with recent versions of the Geographic
 * Markup Language (GML) standard, the IWXXM standard, and OGC Observations and Measurements v2.0 (ISO/DIS 19156).
 * Namespaced units are considered deprecated. We will be aligning API to use the same standards as GML/IWXXM in the future.
 */
UnitId NOAAIon::parseUnit(const QString &unitCode) const
{
    const auto unitsMap = std::map<QString, UnitId>{
        // Simple deprecated "Temperature Unit" string
        {u"F"_s, Fahrenheit},
        {u"C"_s, Celsius},
        // WMO
        {u"wmoUnit:degC"_s, Celsius},
        {u"wmoUnit:percent"_s, Percent},
        {u"wmoUnit:km_h-1"_s, KilometerPerHour},
        {u"wmoUnit:Pa"_s, Pascal},
        {u"wmoUnit:m"_s, Meter},
        {u"wmoUnit:mm"_s, Millimeter},
        {u"wmoUnit:degree_(angle)"_s, Degree},
    };

    QString unit = unitCode;
    unit.replace(u"wmo:"_s, u"wmoUnit:"_s);
    unit.replace(u"uc:"_s, u""_s);

    if (!unitsMap.contains(unit)) {
        qCWarning(WEATHER::ION::NOAA) << "Couldn't parse remote unit" << unitCode;
        return InvalidUnit;
    }

    return unitsMap.at(unit);
}

/* QuantitativeValue
 * https://www.weather.gov/documentation/services-web-api
 */
float NOAAIon::parseQV(const QJsonValue &qv, UnitId destUnit) const
{
    if (qv.isNull() || !qv.isObject()) {
        return qQNaN();
    }

    const float value = qv[u"value"_s].toDouble(qQNaN());
    const UnitId unit = parseUnit(qv[u"unitCode"_s].toString());

    // We don't need or we can't make a conversion
    if (qIsNaN(value) || unit == destUnit || unit == InvalidUnit || destUnit == InvalidUnit) {
        return value;
    }

    return m_converter.convert({value, unit}, destUnit).number();
}

QString NOAAIon::windDirectionFromAngle(float degrees) const
{
    if (qIsNaN(degrees)) {
        return u"VR"_s;
    }

    // We have a discrete set of 16 directions, with resolution of 22.5º
    const std::array<QString, 16> directions{
        u"N"_s,
        u"NNE"_s,
        u"NE"_s,
        u"ENE"_s,
        u"E"_s,
        u"ESE"_s,
        u"SE"_s,
        u"SSE"_s,
        u"S"_s,
        u"SSW"_s,
        u"SW"_s,
        u"WSW"_s,
        u"W"_s,
        u"WNW"_s,
        u"NW"_s,
        u"NNW"_s,
    };
    const int index = qRound(degrees / 22.5) % 16;

    return directions.at(index);
}

void NOAAIon::getForecast()
{
    if (m_weatherData->forecastUrl.isEmpty()) {
        qCWarning(WEATHER::ION::NOAA) << "Cannot request forecast because the URL is missing";
        // If the alerts job also ended with error finish the promise and return.
        if (m_weatherData->isAlertsDataError) {
            m_forecastPromise->finish();
            m_forecastPromise.reset();
            m_weatherData.reset();
        } else {
            // Otherwise set the forecast job error to the inform alerts job
            m_weatherData->isForecastsDataError = true;
        }
        return;
    }

    auto job = requestAPIJob(QUrl(m_weatherData->forecastUrl), &NOAAIon::readForecast);
    m_jobs.insert(job);
}

void NOAAIon::readForecast(KJob *job, const QByteArray &data)
{
    m_jobs.remove(job);

    // we don't stop the forecast job if the warnings job are failed because we can show
    // forecast even without warnings.

    if (m_forecastPromise->isCanceled()) {
        qCDebug(WEATHER::ION::NOAA) << "Forecast fetching cancelled. Return";
        if (m_jobs.isEmpty()) {
            // if cancelled then don't remove unneeded data until last job is finished because otherwise it
            // can lead to not correct forecast data or even crash.
            m_forecastPromise->finish();
            m_forecastPromise.reset();
            m_weatherData.reset();
        }
        return;
    }

    if (job->error()) {
        qCWarning(WEATHER::ION::NOAA) << "Error retrieving data" << job->errorText();
        if (m_jobs.isEmpty()) {
            m_forecastPromise->finish();
            m_forecastPromise.reset();
            m_weatherData.reset();
        } else {
            m_weatherData->isForecastsDataError = true;
        }
        return;
    }

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);

    if (doc.isNull()) {
        qCWarning(WEATHER::ION::NOAA) << "Received invalid JSON data:" << jsonError.errorString();
        if (m_jobs.isEmpty()) {
            m_forecastPromise->finish();
            m_forecastPromise.reset();
            m_weatherData.reset();
        } else {
            m_weatherData->isForecastsDataError = true;
        }
        return;
    }

    const QJsonValue properties = doc[u"properties"_s];
    if (!properties.isObject()) {
        qCWarning(WEATHER::ION::NOAA) << "Malformed forecast report" << doc;
        if (m_jobs.isEmpty()) {
            m_forecastPromise->finish();
            m_forecastPromise.reset();
            m_weatherData.reset();
        } else {
            m_weatherData->isForecastsDataError = true;
        }
        return;
    }

    auto &forecasts = m_weatherData->forecasts;
    forecasts.clear();

    const QJsonArray periods = properties[u"periods"_s].toArray();
    forecasts.reserve(periods.count());

    for (const auto &period : periods) {
        WeatherData::Forecast forecast;

        // Time period. Date and day/night flag
        const QDateTime date = QDateTime::fromString(period[u"startTime"_s].toString(), Qt::ISODate);
        forecast.day = date.date().day();
        forecast.isDayTime = period[u"isDaytime"_s].toBool();

        // The temperature reported is daytime's highest or night's lowest
        // "temperature" can be either an integer (to be deprecated) or a QuantitativeValue
        // Let's use Fahrenheit for now as the default unit for the provider
        const auto tempJson = period[u"temperature"_s];
        float tempF = qQNaN();
        if (tempJson.isObject()) {
            tempF = parseQV(tempJson, Fahrenheit);
        } else {
            const auto temperature = Value(tempJson.toInt(), parseUnit(period[u"temperatureUnit"_s].toString()));
            tempF = m_converter.convert(temperature, Fahrenheit).number();
        }

        if (forecast.isDayTime) {
            forecast.high = tempF;
        } else {
            forecast.low = tempF;
        }

        // Precipitation (%)
        forecast.precipitation = period[u"probabilityOfPrecipitation"_s][u"value"_s].toInt();

        // Weather conditions
        forecast.summary = period[u"shortForecast"].toString();

        forecasts << forecast;
    }

    qCDebug(WEATHER::ION::NOAA) << "Received forecast data:" << forecasts.count() << "periods."
                                << "Starts at night:" << (!forecasts.isEmpty() && !forecasts.first().isDayTime);

    if (m_jobs.isEmpty()) {
        updateWeather();
    }
}

void NOAAIon::getPointsInfo()
{
    const double lat = m_weatherData->stationLatitude;
    const double lon = m_weatherData->stationLongitude;
    if (qIsNaN(lat) || qIsNaN(lon)) {
        qCWarning(WEATHER::ION::NOAA) << "Cannot request grid info because the lat/lon coordinates are missing";
        m_weatherData->isPointsInfoDataError = true;
        return;
    }

    auto job = requestAPIJob(QUrl(u"https://api.weather.gov/points/%1,%2"_s.arg(lat).arg(lon)), &NOAAIon::readPointsInfo);
    m_jobs.insert(job);
}

void NOAAIon::readPointsInfo(KJob *job, const QByteArray &data)
{
    m_jobs.remove(job);

    // If the observation job ended with error then stop processing to avoid malformed forecast
    if (m_weatherData->isObservationDataError) {
        qCDebug(WEATHER::ION::NOAA) << "Error reading observation data. Stop reading points info data";
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        m_weatherData.reset();
        return;
    }

    if (m_forecastPromise->isCanceled()) {
        qCDebug(WEATHER::ION::NOAA) << "Forecast fetching cancelled. Return";
        // if cancelled then don't remove unneeded data until last job is finished because otherwise it
        // can lead to not correct forecast data or even crash.
        if (m_jobs.isEmpty()) {
            m_forecastPromise->finish();
            m_forecastPromise.reset();
            m_weatherData.reset();
        }
        return;
    }

    if (job->error()) {
        qCWarning(WEATHER::ION::NOAA) << "Error retrieving data" << job->errorText();
        if (m_jobs.isEmpty()) {
            m_forecastPromise->finish();
            m_forecastPromise.reset();
            m_weatherData.reset();
        } else {
            m_weatherData->isPointsInfoDataError = true;
        }
        return;
    }

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);

    if (doc.isNull()) {
        qCWarning(WEATHER::ION::NOAA) << "Received invalid JSON data:" << jsonError.errorString();
        if (m_jobs.isEmpty()) {
            m_forecastPromise->finish();
            m_forecastPromise.reset();
            m_weatherData.reset();
        } else {
            m_weatherData->isPointsInfoDataError = true;
        }
        return;
    }

    const auto properties = doc[u"properties"_s];
    if (!properties.isObject()) {
        qCWarning(WEATHER::ION::NOAA) << "Malformed points information" << doc;
        if (m_jobs.isEmpty()) {
            m_forecastPromise->finish();
            m_forecastPromise.reset();
            m_weatherData.reset();
        } else {
            m_weatherData->isObservationDataError = true;
        }
        return;
    }

    m_weatherData->forecastUrl = properties[u"forecast"_s].toString();

    // County ID, used to retrieve alerts
    const QString countyUrl = properties[u"county"_s].toString();
    const QString countyID = countyUrl.split('/'_L1).last();
    m_weatherData->countyID = countyID;

    if (m_jobs.isEmpty()) {
        Q_EMIT observationUpdated();
    }
}

void NOAAIon::getAlerts()
{
    if (m_weatherData->isForecastsDataError) {
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        m_weatherData.reset();
        return;
    }

    // We get the alerts by county because it includes all the events.
    // Using the forecast zone would miss some of them, and the lat/lon point
    // corresponds to the weather station, not necessarily the user location
    const QString &countyID = m_weatherData->countyID;
    if (countyID.isEmpty()) {
        qCWarning(WEATHER::ION::NOAA) << "Cannot request alerts because the county ID is missing";
        m_weatherData->isForecastsDataError = true;
        return;
    }

    auto job = requestAPIJob(QUrl(u"https://api.weather.gov/alerts/active?zone=%1"_s.arg(countyID)), &NOAAIon::readAlerts);
    m_jobs.insert(job);
}

// Helpers to parse warnings
Warnings::PriorityClass mapSeverity(const QString &severity)
{
    if (severity == "Extreme"_L1) {
        return Warnings::Extreme;
    }
    if (severity == "Severe"_L1) {
        return Warnings::High;
    }
    if (severity == "Moderate"_L1) {
        return Warnings::Medium;
    }
    return Warnings::Low;
};

QString formatAlertDescription(QString description)
{
    /* -- Example of an alert's description --
    * WHAT...Minor flooding is occurring and minor flooding is forecast.\n
    \n
    * WHERE...Santee River near Jamestown.\n
    \n
    * WHEN...Until further notice.\n
    \n
    * IMPACTS...At 12.0 feet, several dirt logging roads are impassable.\n
    \n
    * ADDITIONAL DETAILS...\n
    - At 930 PM EST Tuesday, the stage was 11.4 feet.\n
    - Forecast...The river is expected to rise to a crest of 11.7\n
    feet Thursday evening.\n
    - Flood stage is 10.0 feet.\n
    */
    description.replace("* "_L1, "<b>"_L1);
    description.replace("..."_L1, ":</b> "_L1);
    description.replace("\n\n"_L1, "<br/>"_L1);
    description.replace("\n-"_L1, "<br/>-"_L1);
    return description;
}

void NOAAIon::readAlerts(KJob *job, const QByteArray &data)
{
    m_jobs.remove(job);

    if (m_weatherData->isForecastsDataError) {
        qCDebug(WEATHER::ION::NOAA) << "Error reading points info data. Stop reading observation data";
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        m_weatherData.reset();
        return;
    }

    if (m_forecastPromise->isCanceled()) {
        qCDebug(WEATHER::ION::NOAA) << "Forecast fetching cancelled. Return";
        if (m_jobs.isEmpty()) {
            // if cancelled then don't remove unneeded data until last job is finished because otherwise it
            // can lead to not correct forecast data or even crash.
            m_forecastPromise->finish();
            m_forecastPromise.reset();
            m_weatherData.reset();
        }
        return;
    }

    if (job->error()) {
        qCWarning(WEATHER::ION::NOAA) << "Error retrieving data" << job->errorText();
        m_weatherData->isAlertsDataError = true;
        if (m_jobs.isEmpty()) {
            updateWeather();
        }
        return;
    }

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);

    if (doc.isNull()) {
        qCWarning(WEATHER::ION::NOAA) << "Received invalid JSON data:" << jsonError.errorString();
        m_weatherData->isAlertsDataError = true;
        if (m_jobs.isEmpty()) {
            updateWeather();
        }
        return;
    }

    auto &alerts = m_weatherData->alerts;

    const auto features = doc[u"features"_s].toArray();
    qCDebug(WEATHER::ION::NOAA) << u"Received %1 alert/s"_s.arg(features.count());

    for (const auto &alertInfo : features) {
        const auto properties = alertInfo[u"properties"_s];
        if (!properties.isObject()) {
            continue;
        }

        auto alert = WeatherData::Alert();
        alert.startTime = QDateTime::fromString(properties[u"onset"_s].toString(), Qt::ISODate);
        alert.endTime = QDateTime::fromString(properties[u"ends"_s].toString(), Qt::ISODate);
        alert.priority = mapSeverity(properties[u"severity"_s].toString());
        alert.headline = properties[u"parameters"_s][u"NWSheadline"_s][0].toString();
        alert.description = formatAlertDescription(properties[u"description"_s].toString());

        alerts << alert;
    }

    // Sort by higher priority and the lower start time
    std::sort(alerts.begin(), alerts.end(), [](auto a, auto b) {
        if (a.priority != b.priority) {
            return a.priority > b.priority;
        }
        return a.startTime < b.startTime;
    });

    if (m_jobs.isEmpty()) {
        updateWeather();
    }
}

#include "ion_noaa.moc"
#include "moc_ion_noaa.cpp"
