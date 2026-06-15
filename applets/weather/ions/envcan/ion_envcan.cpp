/*
    SPDX-FileCopyrightText: 2007-2011, 2019 Shawn Starr <shawn.starr@rogers.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/* Ion for Environment Canada XML data */

#include "ion_envcan.h"

#include "envcan_debug.h"

#include <KIO/TransferJob>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KUnitConversion/Converter>

#include <QRegularExpression>
#include <QTimeZone>
#include <QUrl>

K_PLUGIN_CLASS_WITH_JSON(EnvCanadaIon, "metadata.json")

using namespace Qt::StringLiterals;

WeatherData::WeatherData()
    : stationLatitude(qQNaN())
    , stationLongitude(qQNaN())
    , temperature(qQNaN())
    , dewpoint(qQNaN())
    , windchill(qQNaN())
    , pressure(qQNaN())
    , visibility(qQNaN())
    , humidity(qQNaN())
    , windSpeed(qQNaN())
    , windGust(qQNaN())
    , normalHigh(qQNaN())
    , normalLow(qQNaN())
    , prevHigh(qQNaN())
    , prevLow(qQNaN())
    , recordHigh(qQNaN())
    , recordLow(qQNaN())
    , recordRain(qQNaN())
    , recordSnow(qQNaN())
{
}

WeatherData::ForecastInfo::ForecastInfo()
    : tempHigh(qQNaN())
    , tempLow(qQNaN())
    , popPrecent(qQNaN())
{
}

// ctor, dtor
EnvCanadaIon::EnvCanadaIon(QObject *parent, const QVariantList &args)
    : Ion(parent)
{
    Q_UNUSED(args);
}

EnvCanadaIon::~EnvCanadaIon()
{
}

Ion::ConditionIcons EnvCanadaIon::conditionIconFromCode(int code) const
{
    switch (code) {
    case 0: // Sunny
        return ClearDay;

    case 1: // Mainly Sunny
        return FewCloudsDay;

    case 2: // Partly Cloudy
        return PartlyCloudyDay;

    case 3: // Mostly Cloudy
        return PartlyCloudyDay;

    case 6: // Light Rain Shower
        return LightRain;

    case 7: // Light Rain Shower and Flurries
        return RainSnow;

    case 8: // Light Flurries
        return Flurries;

    case 10: // Cloudy
        return Overcast;

    case 11: // Precipitation / Squalls
        return Rain;

    case 12: // Rain Shower / Light Rain
    case 13: // Heavy Rain / Rain and Drizzle
        return Rain;

    case 14: // Freezing Rain / Freezing Drizzle
        return FreezingRain;

    case 15: // Rain and Snow / Rain and Flurries
        return RainSnow;

    case 16: // Light Snow
        return LightSnow;

    case 17: // Flurries / Snow
    case 18: // Heavy Flurries / Heavy Snow
        return Snow;

    case 19: // Thunderstorm with Rain
        return Thunderstorm;

    case 23: // Haze
        return Haze;

    case 24: // Fog / Mist / Ice Fog
        return Mist;

    case 25: // Drifting Snow
    case 40: // Blowing Snow
        return Snow;

    case 26: // Ice Crystals
        return Flurries;

    case 27: // Hail / Ice Pellets / Snow Pellets
        return Hail;

    case 28: // Drizzle / Snow Grains
        return LightRain;

    case 30: // Clear
        return ClearNight;

    case 31: // Mainly Clear
        return FewCloudsNight;

    case 32: // Partly Cloudy
        return PartlyCloudyNight;

    case 33: // Mostly Cloudy
        return PartlyCloudyNight;

    case 36: // Light Rain Shower
        return LightRain;

    case 37: // Light Rain Shower and Flurries
        return RainSnow;

    case 38: // Light Flurries
        return Flurries;

    case 39: // Thunderstorm with Rain
        return Thunderstorm;

    case 29: // Not Available
    case 41: // Funnel Cloud
    case 42: // Tornado
    case 43: // Windy
    case 44: // Smoke
    case 45: // Dust Storm / Volcanic Ash
    case 46: // Thunderstorm with Hail
    case 47: // Thunderstorm with Dust Storm
    case 48: // Waterspout
    default:
        return NotAvailable;
    }
}

Ion::ConditionIcons EnvCanadaIon::forecastIconFromCode(int code) const
{
    switch (code) {
    case 0: // Sunny
        return ClearDay;

    case 1: // A Few Clouds
        return FewCloudsDay;

    case 2: // A Mix of Sun and Cloud
    case 3: // Cloudy Periods
    case 22: // A Mix of Sun and Cloud (Day and Night)
        return PartlyCloudyDay;

    case 4: // Increasing Cloudiness
        return Overcast;

    case 5: // Clearing
        return ClearDay;

    case 6: // Chance of Showers
        return ChanceShowersDay;

    case 7: // Chance of Flurries or Rain Showers
        return RainSnow;

    case 8: // Chance of Flurries
        return ChanceSnowDay;

    case 9: // Chance of Thunderstorms
        return ChanceThunderstormDay;

    case 10: // Cloudy / Overcast
        return Overcast;

    case 12: // Showers
        return Showers;

    case 13: // Periods of Rain
        return Rain;

    case 14: // Freezing Rain
        return FreezingRain;

    case 15: // Rain and Snow Mix
        return RainSnow;

    case 16: // Flurries / Wet Snow
        return Flurries;

    case 17: // Snow
    case 18: // Blizzard
    case 40: // Snow and Blowing Snow
        return Snow;

    case 19: // Thunderstorms
        return Thunderstorm;

    case 23: // Haze
        return Haze;

    case 24: // Fog
        return Mist;

    case 27: // Ice Pellets
        return Hail;

    case 28: // Drizzle
        return LightRain;

    case 30: // Clear
        return ClearNight;

    case 31: // A Few Clouds
        return FewCloudsNight;

    case 32: // Cloudy Periods
    case 33: // Cloudy
        return PartlyCloudyNight;

    case 34: // Increasing Cloudiness
        return Overcast;

    case 35: // Clearing
        return ClearNight;

    case 36: // Chance of Showers
        return ChanceShowersNight;

    case 37: // Chance of Snow or Rain
        return RainSnow;

    case 38: // Chance of Light Snow
        return ChanceSnowNight;

    case 39: // Chance of Thunderstorms
        return ChanceThunderstormNight;

    case 29: // Not Available
    case 43: // Windy
    case 44: // Smoke
    default:
        return NotAvailable;
    }
}

void EnvCanadaIon::clearForecastData()
{
    qCDebug(WEATHER::ION::ENVCAN) << " clear forecast data";
    m_territoryName.clear();
    m_cityCode.clear();
    m_forecastPromise.reset();
    m_xmlForecast.reset();
    m_weatherData.reset();
}

void EnvCanadaIon::clearLocationData()
{
    qCDebug(WEATHER::ION::ENVCAN) << " clear location data";
    m_xmlSetup.reset();
    m_searchString.clear();
    m_locationPromise.reset();
}

void EnvCanadaIon::validate(const QString &source)
{
    if (m_locationPromise->isCanceled()) {
        clearLocationData();
        return;
    }

    auto locations = std::make_shared<Locations>();

    qCDebug(WEATHER::ION::ENVCAN) << "start adding locations";

    QString sourceNormalized = source.toUpper();
    for (auto it = m_places.constBegin(); it != m_places.constEnd(); ++it) {
        if (it.key().toUpper().contains(sourceNormalized)) {
            Location location;
            location.setDisplayName(it.key());
            location.setStation(it->territoryName);
            location.setCode(it->cityCode);
            location.setPlaceInfo(it->territoryName + u"|"_s + it->cityCode);
            locations->addLocation(location);
        }
    }

    qCDebug(WEATHER::ION::ENVCAN) << " validated locations. Total locations:" << locations->rowCount() << " .Return";

    m_locationPromise->addResult(locations);
}

void EnvCanadaIon::findPlaces(std::shared_ptr<QPromise<std::shared_ptr<Locations>>> promise, const QString &searchString)
{
    m_locationPromise = promise;

    if (m_locationPromise->isCanceled()) {
        m_forecastPromise->finish();
        clearLocationData();
        return;
    }

    if (m_places.isEmpty()) {
        m_searchString = searchString;
        // If network is down, we need to spin and wait
        const QUrl url(QStringLiteral("https://dd.weather.gc.ca/today/citypage_weather/siteList.xml"));
        qCDebug(WEATHER::ION::ENVCAN) << "Fetching station list:" << url;

        KIO::TransferJob *getJob = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);

        m_xmlSetup = std::make_shared<QXmlStreamReader>();
        connect(getJob, &KIO::TransferJob::data, this, &EnvCanadaIon::places_slotDataArrived);
        connect(getJob, &KJob::result, this, &EnvCanadaIon::places_slotJobFinished);
    } else {
        validate(searchString);
        m_locationPromise->finish();
        clearLocationData();
    }
}

void EnvCanadaIon::fetchForecast(std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> promise, const QString &placeInfo)
{
    m_forecastPromise = promise;

    if (m_forecastPromise->isCanceled()) {
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    // place_name|id - Triggers receiving weather of place
    const QList<QString> info = placeInfo.split('|'_L1);

    qCDebug(WEATHER::ION::ENVCAN) << "Start receiving forecast for info " << placeInfo;

    // Support the old format for receiving a forecast
    if (info.size() == 3 && info[1] == "weather"_L1 && !m_isLegacy) {
        m_isLegacy = true;
        if (m_places.isEmpty()) {
            // If network is down, we need to spin and wait
            const QUrl url(QStringLiteral("https://dd.weather.gc.ca/today/citypage_weather/siteList.xml"));
            qCDebug(WEATHER::ION::ENVCAN) << "Fetching station list:" << url;

            KIO::TransferJob *getJob = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);

            m_xmlSetup = std::make_shared<QXmlStreamReader>();
            connect(getJob, &KIO::TransferJob::data, this, &EnvCanadaIon::places_slotDataArrived);
            connect(getJob, &KJob::result, this, &EnvCanadaIon::places_slotJobFinished);
            m_legacyPlaceInfo = info[2];
            return;
        }
        if (auto it = m_places.constFind(placeInfo); it != m_places.constEnd()) {
            fetchForecast(m_forecastPromise, it->territoryName + u"|"_s + it->cityCode);
            return;
        }
        m_isLegacy = false;
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    if (info.count() > 2) {
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    m_territoryName = info.first();
    m_cityCode = info.last();

    if (m_territoryName.isEmpty() && m_cityCode.isEmpty()) {
        m_forecastPromise->finish();
        clearForecastData();
        return;
    }

    m_xmlForecast = std::make_shared<QXmlStreamReader>();
    m_weatherData = std::make_shared<WeatherData>();

    m_weatherData->shortTerritoryName = m_territoryName;

    getWeatherData();
}

void EnvCanadaIon::places_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job)

    Q_ASSERT(m_locationPromise);

    if (data.isEmpty()) {
        return;
    }

    if (!m_isLegacy && m_locationPromise->isCanceled()) {
        return;
    }

    // Send to xml.
    m_xmlSetup->addData(data);
}

void EnvCanadaIon::places_slotJobFinished(KJob *job)
{
    Q_UNUSED(job)

    Q_ASSERT(m_locationPromise);

    qCDebug(WEATHER::ION::ENVCAN) << "Location job finished";

    if (m_isLegacy) {
        readXMLSetup();
        if (auto it = m_places.constFind(m_legacyPlaceInfo); it != m_places.constEnd()) {
            qCDebug(WEATHER::ION::ENVCAN) << "Start fetching default forecast";
            fetchForecast(m_forecastPromise, it->territoryName + u"|"_s + it->cityCode);
            return;
        }
        m_isLegacy = false;
        m_forecastPromise->finish();
        clearForecastData();
        return;
    }

    if (m_locationPromise->isCanceled()) {
        m_locationPromise->finish();
        clearLocationData();
        return;
    }

    readXMLSetup();

    validate(m_searchString);

    m_locationPromise->finish();
    clearLocationData();
}

// The weather URL has a dynamic name and path depending on its timestamp:
// https://dd.weather.gc.ca/today/citypage_weather/{PROV}/{HH}/{YYYYMMDD}T{HHmmss.sss}Z_MSC_CitypageWeather_{SiteCode}_en.xml
// This method is called iteratively 3 times to get the URL and then the weather report
void EnvCanadaIon::getWeatherData()
{
    WeatherData::UrlInfo &info = m_weatherData->urlInfo;

    info.requests++;
    if (info.requests > 3) {
        qCWarning(WEATHER::ION::ENVCAN) << "Too many requests to find the weather URL";
        return;
    }

    // We get the place info from the stations list
    if (info.cityCode.isEmpty()) {
        info.province = m_territoryName;
        info.cityCode = m_cityCode;
    }

    // 1. Base URL, on the territory dir, to get the list of hours
    QString url = u"https://dd.weather.gc.ca/today/citypage_weather/%1/"_s.arg(info.province);
    // 2. When we know the hour folder, we check for the weather report files
    if (!info.hours.isEmpty()) {
        url += info.hours.at(info.hourIndex) + u"/"_s;
    }
    // 3. Now we have the full information to compose the URL
    if (!info.fileName.isEmpty()) {
        url += info.fileName;
    }

    qCDebug(WEATHER::ION::ENVCAN) << "Fetching weather URL:" << url;

    KIO::TransferJob *getJob = KIO::get(QUrl(url), KIO::Reload, KIO::HideProgressInfo);

    m_xmlForecast = std::make_shared<QXmlStreamReader>();

    connect(getJob, &KIO::TransferJob::data, this, &EnvCanadaIon::forecast_slotDataArrived);
    connect(getJob, &KJob::result, this, &EnvCanadaIon::forecast_slotJobFinished);
}

void EnvCanadaIon::forecast_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job);

    if (m_forecastPromise->isCanceled()) {
        return;
    }

    if (data.isEmpty()) {
        return;
    }

    // Remove the HTML doctype line from XML parsing
    if (data.startsWith("<!DOCTYPE"_ba)) {
        int newLinePos = data.indexOf('\n');
        m_xmlForecast->addData(QByteArrayView(data).slice(newLinePos + 1));
        return;
    }

    m_xmlForecast->addData(data);
}

void EnvCanadaIon::forecast_slotJobFinished(KJob *job)
{
    if (m_forecastPromise->isCanceled()) {
        m_forecastPromise->finish();
        clearForecastData();
        return;
    }

    if (job->error()) {
        m_forecastPromise->finish();
        clearForecastData();
        return;
    }

    readXMLData(*m_xmlForecast);
}

// Parse the city list and store into a QMap
void EnvCanadaIon::readXMLSetup()
{
    QString territory;
    QString code;
    QString cityName;

    while (!m_xmlSetup->atEnd()) {
        m_xmlSetup->readNext();

        const auto elementName = m_xmlSetup->name();

        if (m_xmlSetup->isStartElement()) {
            // XML ID code to match filename
            if (elementName == QLatin1String("site")) {
                code = m_xmlSetup->attributes().value(u"code"_s).toString();
            }

            if (elementName == QLatin1String("nameEn")) {
                cityName = m_xmlSetup->readElementText(); // Name of cities
            }

            if (elementName == QLatin1String("provinceCode")) {
                territory = m_xmlSetup->readElementText(); // Provinces/Territory list
            }
        }

        if (m_xmlSetup->isEndElement() && elementName == QLatin1String("site")) {
            XMLMapInfo info;
            QString tmp = cityName + u", "_s + territory; // Build the key name.

            // Set the mappings
            info.cityCode = code;
            info.territoryName = territory;
            info.cityName = cityName;

            // Set the string list, we will use for the applet to display the available cities.
            m_places[tmp] = info;
        }
    }
}

void EnvCanadaIon::parseWeatherSite(WeatherData &data, QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("license")) {
                data.creditUrl = xml.readElementText();
            } else if (elementName == QLatin1String("location")) {
                parseLocations(data, xml);
            } else if (elementName == QLatin1String("warnings")) {
                // Cleanup warning list on update
                data.warnings.clear();
                parseWarnings(data, xml);
            } else if (elementName == QLatin1String("currentConditions")) {
                parseConditions(data, xml);
            } else if (elementName == QLatin1String("forecastGroup")) {
                // Clean up forecast list on update
                data.forecasts.clear();
                parseWeatherForecast(data, xml);
            } else if (elementName == QLatin1String("yesterdayConditions")) {
                parseYesterdayWeather(data, xml);
            } else if (elementName == QLatin1String("riseSet")) {
                parseAstronomicals(data, xml);
            } else if (elementName == QLatin1String("almanac")) {
                parseWeatherRecords(data, xml);
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

// Parse Weather data main loop, from here we have to descend into each tag pair
void EnvCanadaIon::readXMLData(QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("siteData")) {
                parseWeatherSite(*m_weatherData, xml);
            } else if (xml.name() == QLatin1String("html")) {
                if (!xml.hasError()) {
                    parseDirListing(m_weatherData->urlInfo, xml);
                    getWeatherData();
                    return;
                }
            } else {
                parseUnknownElement(xml);
            }
        }
    }

    // update and return forecast if no error
    if (!xml.hasError()) {
        updateWeather();
    }

    m_forecastPromise->finish();
    clearForecastData();
}

void EnvCanadaIon::parseDirListing(WeatherData::UrlInfo &info, QXmlStreamReader &xml)
{
    const bool expectingFileNames = !info.hours.isEmpty();

    while (!xml.atEnd()) {
        xml.readNext();

        // We are parsing a directory listing with files or folders as hyperlinks
        if (xml.isStartElement() && xml.name() == "a"_L1) {
            QString item = xml.attributes().value(u"href"_s).toString().trimmed();

            // Check for hour folders
            if (!expectingFileNames && item.endsWith('/'_L1)) {
                item.slice(0, item.length() - 1);

                bool isHour = false;
                item.toInt(&isHour);
                if (isHour) {
                    info.hours.prepend(item);
                }
                continue;
            }

            // Check just for files that match our city code en English language
            if (item.endsWith(u"%1_en.xml"_s.arg(info.cityCode))) {
                info.fileName = item;
            }
        }
    }

    // Sort hours in reverse order (more recent first)
    if (!expectingFileNames && !info.hours.isEmpty()) {
        std::sort(info.hours.begin(), info.hours.end(), [](const auto &a, const auto &b) {
            return a.toInt() > b.toInt();
        });
    }

    // If we didn't find the filename in the current hour folder
    // set up a new requests to search it on the previous one
    if (expectingFileNames && info.fileName.isEmpty()) {
        if (info.hourIndex < info.hours.count()) {
            info.hourIndex++;
            info.requests--;
        }
    }
}

void EnvCanadaIon::parseFloat(float &value, QXmlStreamReader &xml)
{
    bool ok = false;
    const float result = xml.readElementText().toFloat(&ok);
    if (ok) {
        value = result;
    }
}

void EnvCanadaIon::parseDateTime(WeatherData &data, QXmlStreamReader &xml, std::shared_ptr<WeatherData::WeatherEvent> event)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("dateTime"));

    const QString dateType = xml.attributes().value(u"name"_s).toString();
    const QString dateZone = xml.attributes().value(u"zone"_s).toString();
    const QString utcOffsetStr = xml.attributes().value(u"UTCOffset"_s).toString();

    if (dateZone == QLatin1String("UTC") || dateType == QLatin1String("xmlCreation")) {
        return;
    }

    QString timestamp;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        const auto elementName = xml.name();

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("year")) {
                xml.readElementText();
            } else if (elementName == QLatin1String("month")) {
                xml.readElementText();
            } else if (elementName == QLatin1String("day")) {
                xml.readElementText();
            } else if (elementName == QLatin1String("hour"))
                xml.readElementText();
            else if (elementName == QLatin1String("minute"))
                xml.readElementText();
            else if (elementName == QLatin1String("timeStamp"))
                timestamp = xml.readElementText();
            else if (elementName == QLatin1String("textSummary")) {
                xml.readElementText();
            }
        }
    }

    if (timestamp.isEmpty()) {
        return;
    }

    QDateTime dateTime = QDateTime::fromString(timestamp, QStringLiteral("yyyyMMddHHmmss"));
    if (!dateTime.isValid()) {
        return;
    }

    QTimeZone timeZone;

    if (!dateZone.isEmpty()) {
        timeZone = QTimeZone(dateZone.toUtf8());
    }

    if (!timeZone.isValid()) {
        bool ok = false;
        const int offsetHours = utcOffsetStr.toInt(&ok);
        if (ok) {
            timeZone = QTimeZone(offsetHours * 3600);
        }
    }

    if (timeZone.isValid()) {
        dateTime.setTimeZone(timeZone);
    }

    if (dateType == QLatin1String("eventIssue")) {
        if (event) {
            event->timestamp = dateTime;
        }
    } else if (dateType == QLatin1String("observation")) {
        data.observationDateTime = dateTime;
    } else if (dateType == QLatin1String("forecastIssue")) {
        data.forecastTimestamp = dateTime;
    } else if (dateType == QLatin1String("sunrise")) {
        data.sunriseTimestamp = dateTime;
    } else if (dateType == QLatin1String("sunset")) {
        data.sunsetTimestamp = dateTime;
    } else if (dateType == QLatin1String("moonrise")) {
        data.moonriseTimestamp = dateTime;
    } else if (dateType == QLatin1String("moonset")) {
        data.moonsetTimestamp = dateTime;
    }
}

void EnvCanadaIon::parseLocations(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("location"));

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        const auto elementName = xml.name();

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("country")) {
                data.countryName = xml.readElementText();
            } else if (elementName == QLatin1String("province") || elementName == QLatin1String("territory")) {
                data.longTerritoryName = xml.readElementText();
            } else if (elementName == QLatin1String("name")) {
                data.cityName = xml.readElementText();
            } else if (elementName == QLatin1String("region")) {
                data.regionName = xml.readElementText();
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

void EnvCanadaIon::parseWindInfo(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("wind"));

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        const auto elementName = xml.name();

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("speed")) {
                parseFloat(data.windSpeed, xml);
            } else if (elementName == QLatin1String("gust")) {
                parseFloat(data.windGust, xml);
            } else if (elementName == QLatin1String("direction")) {
                data.windDirection = xml.readElementText();
            } else if (elementName == QLatin1String("bearing")) {
                data.windDegrees = xml.attributes().value(u"degrees"_s).toString();
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

float EnvCanadaIon::parseCoordinate(QStringView coord) const
{
    // Coordinates are in form of "64.52N" or "105.23W"
    const QRegularExpression coord_re(QStringLiteral("([0-9\\.]+)([NSEW])"));
    const QRegularExpressionMatch match = coord_re.matchView(coord);
    if (!match.hasMatch()) {
        return qQNaN();
    }

    bool ok = false;
    const float value = match.captured(1).toFloat(&ok);
    if (!ok) {
        return qQNaN();
    }

    const bool isNegative = (match.captured(2) == u"S"_s || match.captured(2) == u"W"_s);
    return isNegative ? -value : value;
};

void EnvCanadaIon::parseConditions(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("currentConditions"));

    // Reset all the condition properties
    data.temperature = qQNaN();
    data.dewpoint = qQNaN();
    data.condition = i18n("N/A");
    data.humidex.clear();
    data.stationID = i18n("N/A");
    data.stationLatitude = qQNaN();
    data.stationLongitude = qQNaN();
    data.pressure = qQNaN();
    data.visibility = qQNaN();
    data.humidity = qQNaN();
    data.windSpeed = qQNaN();
    data.windGust = qQNaN();
    data.windDirection = QString();
    data.windDegrees = QString();

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("currentConditions"))
            break;

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("station")) {
                data.stationID = xml.attributes().value(u"code"_s).toString();
                data.stationLatitude = parseCoordinate(xml.attributes().value(u"lat"_s).toString());
                data.stationLongitude = parseCoordinate(xml.attributes().value(u"lon"_s).toString());
            } else if (elementName == QLatin1String("dateTime")) {
                parseDateTime(data, xml);
            } else if (elementName == QLatin1String("condition")) {
                data.condition = xml.readElementText().trimmed();
            } else if (elementName == QLatin1String("iconCode")) {
                bool isSuccessful = false;
                int iconCode = xml.readElementText().toInt(&isSuccessful);
                if (isSuccessful) {
                    data.iconName = getWeatherIcon(conditionIconFromCode(iconCode));
                }
            } else if (elementName == QLatin1String("temperature")) {
                // prevent N/A text to result in 0.0 value
                parseFloat(data.temperature, xml);
            } else if (elementName == QLatin1String("dewpoint")) {
                // prevent N/A text to result in 0.0 value
                parseFloat(data.dewpoint, xml);
            } else if (elementName == QLatin1String("humidex")) {
                data.humidex = xml.readElementText();
            } else if (elementName == QLatin1String("windChill")) {
                // prevent N/A text to result in 0.0 value
                parseFloat(data.windchill, xml);
            } else if (elementName == QLatin1String("pressure")) {
                data.pressureTendency = xml.attributes().value(u"tendency"_s).toString();
                if (data.pressureTendency.isEmpty()) {
                    data.pressureTendency = u"steady"_s;
                }
                parseFloat(data.pressure, xml);
            } else if (elementName == QLatin1String("visibility")) {
                parseFloat(data.visibility, xml);
            } else if (elementName == QLatin1String("relativeHumidity")) {
                parseFloat(data.humidity, xml);
            } else if (elementName == QLatin1String("wind")) {
                parseWindInfo(data, xml);
            }
        }
    }
}

void EnvCanadaIon::parseWarnings(WeatherData &data, QXmlStreamReader &xml)
{
    auto warning = std::make_shared<WeatherData::WeatherEvent>();

    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("warnings"));
    QString eventURL = xml.attributes().value(u"url"_s).toString();

    // envcan provides three type of events: 'warning', 'watch' and 'advisory'
    const auto mapToPriority = [](const QString &type) {
        if (type == QLatin1String("warning")) {
            return Warnings::High;
        }
        if (type == QLatin1String("watch")) {
            return Warnings::Medium;
        }
        return Warnings::Low;
    };

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("warnings")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("dateTime")) {
                parseDateTime(data, xml, warning);
                if (!warning->timestamp.isValid() && !warning->url.isEmpty()) {
                    data.warnings.append(warning);
                    warning = std::make_shared<WeatherData::WeatherEvent>();
                }
            } else if (elementName == QLatin1String("event")) {
                // Append new event to list.
                warning->url = eventURL;
                warning->description = xml.attributes().value(u"description"_s).toString();
                warning->priority = mapToPriority(xml.attributes().value(u"type"_s).toString());
            } else {
                if (xml.name() != QLatin1String("dateTime")) {
                    parseUnknownElement(xml);
                }
            }
        }
    }
}

void EnvCanadaIon::parseWeatherForecast(WeatherData &data, QXmlStreamReader &xml)
{
    auto forecast = std::make_shared<WeatherData::ForecastInfo>();
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("forecastGroup"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("forecastGroup")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("dateTime")) {
                parseDateTime(data, xml);
            } else if (elementName == QLatin1String("regionalNormals")) {
                parseRegionalNormals(data, xml);
            } else if (elementName == QLatin1String("forecast")) {
                parseForecast(data, xml, forecast);
                forecast = std::make_shared<WeatherData::ForecastInfo>();
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

void EnvCanadaIon::parseRegionalNormals(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("regionalNormals"));

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        const auto elementName = xml.name();

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("textSummary")) {
                xml.readElementText();
            } else if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("high")) {
                // prevent N/A text to result in 0.0 value
                parseFloat(data.normalHigh, xml);
            } else if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("low")) {
                // prevent N/A text to result in 0.0 value
                parseFloat(data.normalLow, xml);
            }
        }
    }
}

void EnvCanadaIon::parseForecast(WeatherData &data, QXmlStreamReader &xml, std::shared_ptr<WeatherData::ForecastInfo> forecast)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("forecast"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("forecast")) {
            data.forecasts.append(forecast);
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("period")) {
                forecast->forecastPeriod = xml.attributes().value(u"textForecastName"_s).toString();
                forecast->forecastTimestamp = dateTimeForForecastPeriod(forecast->forecastPeriod, data.forecastTimestamp.date());
            } else if (elementName == QLatin1String("textSummary")) {
                forecast->forecastSummary = xml.readElementText();
            } else if (elementName == QLatin1String("abbreviatedForecast")) {
                parseShortForecast(forecast, xml);
            } else if (elementName == QLatin1String("temperatures")) {
                parseForecastTemperatures(forecast, xml);
            } else if (elementName == QLatin1String("winds")) {
                parseWindForecast(forecast, xml);
            } else if (elementName == QLatin1String("precipitation")) {
                parsePrecipitationForecast(forecast, xml);
            } else if (elementName == QLatin1String("uv")) {
                data.UVRating = xml.attributes().value(u"category"_s).toString();
                parseUVIndex(data, xml);
                // else if (elementName == QLatin1String("frost")) { FIXME: Wait until winter to see what this looks like.
                //  parseFrost(xml, forecast);
            } else {
                if (elementName != QLatin1String("forecast")) {
                    parseUnknownElement(xml);
                }
            }
        }
    }
}

void EnvCanadaIon::parseShortForecast(std::shared_ptr<WeatherData::ForecastInfo> forecast, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("abbreviatedForecast"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("abbreviatedForecast")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("iconCode")) {
                bool isSuccessful = false;
                int iconCode = xml.readElementText().toInt(&isSuccessful);
                if (isSuccessful) {
                    forecast->iconName = getWeatherIcon(forecastIconFromCode(iconCode));
                }
            } else if (elementName == QLatin1String("pop")) {
                parseFloat(forecast->popPrecent, xml);
            } else if (elementName == QLatin1String("textSummary")) {
                forecast->shortForecast = xml.readElementText();
            }
        }
    }
}

void EnvCanadaIon::parseUVIndex(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("uv"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("uv")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("index")) {
                data.UVIndex = xml.readElementText();
            }
            if (elementName == QLatin1String("textSummary")) {
                xml.readElementText();
            }
        }
    }
}

void EnvCanadaIon::parseForecastTemperatures(std::shared_ptr<WeatherData::ForecastInfo> forecast, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("temperatures"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("temperatures")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("low")) {
                parseFloat(forecast->tempLow, xml);
            } else if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("high")) {
                parseFloat(forecast->tempHigh, xml);
            } else if (elementName == QLatin1String("textSummary")) {
                xml.readElementText();
            }
        }
    }
}

void EnvCanadaIon::parsePrecipitationForecast(std::shared_ptr<WeatherData::ForecastInfo> forecast, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("precipitation"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("precipitation")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("textSummary")) {
                forecast->precipForecast = xml.readElementText();
            } else if (elementName == QLatin1String("precipType")) {
                forecast->precipType = xml.readElementText();
            } else if (elementName == QLatin1String("accumulation")) {
                parsePrecipTotals(forecast, xml);
            }
        }
    }
}

void EnvCanadaIon::parsePrecipTotals(std::shared_ptr<WeatherData::ForecastInfo> forecast, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("accumulation"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("accumulation")) {
            break;
        }

        if (elementName == QLatin1String("name")) {
            xml.readElementText();
        } else if (elementName == QLatin1String("amount")) {
            forecast->precipTotalExpected = xml.readElementText();
        }
    }
}

void EnvCanadaIon::parseWindForecast(std::shared_ptr<WeatherData::ForecastInfo> forecast, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("winds"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("winds")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("textSummary")) {
                forecast->windForecast = xml.readElementText();
            } else {
                if (xml.name() != QLatin1String("winds")) {
                    parseUnknownElement(xml);
                }
            }
        }
    }
}

void EnvCanadaIon::parseYesterdayWeather(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("yesterdayConditions"));

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        const auto elementName = xml.name();

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("high")) {
                parseFloat(data.prevHigh, xml);
            } else if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("low")) {
                parseFloat(data.prevLow, xml);
            } else if (elementName == QLatin1String("precip")) {
                data.prevPrecipType = xml.attributes().value(u"units"_s).toString();
                if (data.prevPrecipType.isEmpty()) {
                    data.prevPrecipType = QString::number(KUnitConversion::NoUnit);
                }
                parseFloat(data.prevPrecipTotal, xml);
            }
        }
    }
}

void EnvCanadaIon::parseWeatherRecords(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("almanac"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("almanac")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("extremeMax")) {
                parseFloat(data.recordHigh, xml);
            } else if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("extremeMin")) {
                parseFloat(data.recordLow, xml);
            } else if (elementName == QLatin1String("precipitation") && xml.attributes().value(u"class"_s) == QLatin1String("extremeRainfall")) {
                parseFloat(data.recordRain, xml);
            } else if (elementName == QLatin1String("precipitation") && xml.attributes().value(u"class"_s) == QLatin1String("extremeSnowfall")) {
                parseFloat(data.recordSnow, xml);
            }
        }
    }
}

void EnvCanadaIon::parseAstronomicals(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("riseSet"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("riseSet")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("disclaimer")) {
                xml.readElementText();
            } else if (elementName == QLatin1String("dateTime")) {
                parseDateTime(data, xml);
            }
        }
    }
}

// handle when no XML tag is found
void EnvCanadaIon::parseUnknownElement(QXmlStreamReader &xml) const
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

QString EnvCanadaIon::updateForecastPeriod(const std::shared_ptr<WeatherData::ForecastInfo> &info)
{
    QString forecastPeriod = info->forecastPeriod;

    if (forecastPeriod.isEmpty()) {
        forecastPeriod = i18n("N/A");
    } else {
        // We need to shortform the week day strings and remove the "night" word because FutureDays already
        // set "day" and "night" labels.
        const QString today = i18n("Today");
        forecastPeriod.replace(QStringLiteral("Today"), today);
        forecastPeriod.replace(QStringLiteral("Tonight"), today);
        forecastPeriod.replace(QStringLiteral("night"), u""_s);
        forecastPeriod.replace(QStringLiteral("Saturday"), i18nc("Short for Saturday", "Sat"));
        forecastPeriod.replace(QStringLiteral("Sunday"), i18nc("Short for Sunday", "Sun"));
        forecastPeriod.replace(QStringLiteral("Monday"), i18nc("Short for Monday", "Mon"));
        forecastPeriod.replace(QStringLiteral("Tuesday"), i18nc("Short for Tuesday", "Tue"));
        forecastPeriod.replace(QStringLiteral("Wednesday"), i18nc("Short for Wednesday", "Wed"));
        forecastPeriod.replace(QStringLiteral("Thursday"), i18nc("Short for Thursday", "Thu"));
        forecastPeriod.replace(QStringLiteral("Friday"), i18nc("Short for Friday", "Fri"));
    }

    return forecastPeriod;
}

QDateTime EnvCanadaIon::dateTimeForForecastPeriod(const QString &periodName, const QDate &issueDate) const
{
    // EnvCan does not provide dates for individual forecast periods, only the
    // forecast start day, so calculate them ourselves.
    QString normalized = periodName.trimmed();

    bool isNight = false;

    // Environment Canada defines daytime forecasts as 06:00-18:00 and
    // nighttime forecasts as 18:00-06:00.
    static const auto daytime = QTime(6, 0);
    static const auto nighttime = QTime(18, 0);

    if (normalized.compare(u"Today"_s, Qt::CaseInsensitive) == 0) {
        return QDateTime(issueDate, isNight ? nighttime : daytime);
    }

    if (normalized.compare(u"Tonight"_s, Qt::CaseInsensitive) == 0) {
        return QDateTime(issueDate, isNight ? nighttime : daytime);
    }

    if (normalized.endsWith(u" night"_s, Qt::CaseInsensitive)) {
        isNight = true;
        // Get the day name. Envcan returns xml where forecasts have textForecastName `Dayname`
        // for day forecasts and `Dayname night` for night forecasts. so trim the `night` and save
        //  only the day name.
        normalized = normalized.split(u" "_s)[0];
    }

    static const QHash<QString, Qt::DayOfWeek> weekdays{
        {u"Monday"_s, Qt::Monday},
        {u"Tuesday"_s, Qt::Tuesday},
        {u"Wednesday"_s, Qt::Wednesday},
        {u"Thursday"_s, Qt::Thursday},
        {u"Friday"_s, Qt::Friday},
        {u"Saturday"_s, Qt::Saturday},
        {u"Sunday"_s, Qt::Sunday},
    };

    const auto it = weekdays.find(normalized);
    if (it == weekdays.end()) {
        qCWarning(WEATHER::ION::ENVCAN) << "Unknown forecast period:" << periodName;
        return {};
    }

    const int issueDay = issueDate.dayOfWeek();
    const int targetDay = static_cast<int>(*it);

    int daysToAdd = targetDay - issueDay;
    // The requested weekday has already passed this week, so use its next
    // occurrence. This is sufficient because EnvCan forecasts span at most one week.
    if (daysToAdd <= 0) {
        daysToAdd += 7;
    }

    const QDate targetDate = issueDate.addDays(daysToAdd);

    return QDateTime(targetDate, isNight ? nighttime : daytime);
}

FutureForecast EnvCanadaIon::forecastInfoToFutureForecast(const std::shared_ptr<WeatherData::ForecastInfo> &info)
{
    const QString shortForecast = info->shortForecast.isEmpty() ? i18n("N/A") : i18nc("weather forecast", info->shortForecast.toUtf8().data());

    FutureForecast futureForecast;

    futureForecast.setConditionIcon(info->iconName);
    futureForecast.setCondition(shortForecast);
    if (!qIsNaN(info->tempHigh)) {
        futureForecast.setHighTemp(info->tempHigh);
    }
    if (!qIsNaN(info->tempLow)) {
        futureForecast.setLowTemp(info->tempLow);
    }
    if (!qIsNaN(info->popPrecent)) {
        futureForecast.setConditionProbability(info->popPrecent);
    }

    return futureForecast;
}

void EnvCanadaIon::updateWeather()
{
    m_weatherData->isNight = isNightTime(m_weatherData->observationDateTime, m_weatherData->stationLatitude, m_weatherData->stationLongitude);

    auto forecast = std::make_shared<Forecast>();

    MetaData metaData;

    metaData.setCredit(i18nc("credit line, keep string short", "Data from Environment and Climate Change\302\240Canada"));
    metaData.setCreditURL(m_weatherData->creditUrl);
    metaData.setTemperatureUnit(KUnitConversion::Celsius);
    metaData.setPressureUnit(KUnitConversion::Kilopascal);
    metaData.setVisibilityUnit(KUnitConversion::Kilometer);
    metaData.setHumidityUnit(KUnitConversion::Percent);
    metaData.setWindSpeedUnit(KUnitConversion::KilometerPerHour);
    metaData.setRainfallUnit(KUnitConversion::Millimeter);
    metaData.setSnowfallUnit(KUnitConversion::Centimeter);
    if (!m_weatherData->prevPrecipType.isEmpty()) {
        if (m_weatherData->prevPrecipType == QLatin1String("mm")) {
            metaData.setPrecipUnit(KUnitConversion::Millimeter);
        } else if (m_weatherData->prevPrecipType == QLatin1String("cm")) {
            metaData.setPrecipUnit(KUnitConversion::Centimeter);
        }
    }

    forecast->setMetadata(metaData);

    Station station;

    station.setCountry(m_weatherData->countryName);
    station.setPlace(m_weatherData->cityName + u", "_s + m_weatherData->shortTerritoryName);
    station.setRegion(m_weatherData->regionName);
    station.setStation(m_weatherData->stationID.isEmpty() ? i18n("N/A") : m_weatherData->stationID.toUpper());

    if (!qIsNaN(m_weatherData->stationLatitude) && !qIsNaN(m_weatherData->stationLongitude)) {
        station.setCoordinates(m_weatherData->stationLatitude, m_weatherData->stationLongitude);
    }

    if (m_isLegacy) {
        station.setNewPlaceInfo(m_territoryName + u"|"_s + m_cityCode);
    }

    forecast->setStation(station);

    LastObservation lastObservation;

    // Real weather - Current conditions
    if (m_weatherData->observationDateTime.isValid()) {
        lastObservation.setObservationTimestamp(m_weatherData->observationDateTime);
    }

    if (!m_weatherData->condition.isEmpty()) {
        lastObservation.setCurrentConditions(i18nc("weather condition", m_weatherData->condition.toUtf8().data()));
    }

    // Use day or night condition icon mappings based on whether it's night
    lastObservation.setConditionIcon(m_weatherData->iconName);

    if (!qIsNaN(m_weatherData->temperature)) {
        lastObservation.setTemperature(m_weatherData->temperature);
    }
    if (!qIsNaN(m_weatherData->windchill)) {
        lastObservation.setWindchill(m_weatherData->windchill);
    }
    if (!m_weatherData->humidex.isEmpty()) {
        lastObservation.setHumidex(m_weatherData->humidex);
    }

    if (!qIsNaN(m_weatherData->dewpoint)) {
        lastObservation.setDewpoint(m_weatherData->dewpoint);
    }

    if (!qIsNaN(m_weatherData->pressure)) {
        lastObservation.setPressure(m_weatherData->pressure);
        if (m_weatherData->pressureTendency == u"rising"_s) {
            lastObservation.setPressureTendency(i18n("Rising"));
        } else if (m_weatherData->pressureTendency == u"falling"_s) {
            lastObservation.setPressureTendency(i18n("Falling"));
        } else {
            lastObservation.setPressureTendency(i18n("Steady"));
        }
    }

    if (!qIsNaN(m_weatherData->visibility)) {
        lastObservation.setVisibility(m_weatherData->visibility);
    }

    if (!qIsNaN(m_weatherData->humidity)) {
        lastObservation.setHumidity(m_weatherData->humidity);
    }

    if (!qIsNaN(m_weatherData->windSpeed)) {
        lastObservation.setWindSpeed(m_weatherData->windSpeed);
    }
    if (!qIsNaN(m_weatherData->windGust)) {
        lastObservation.setWindGust(m_weatherData->windGust);
    }

    if (!qIsNaN(m_weatherData->windSpeed) || !qIsNaN(m_weatherData->windGust)) { }

    if (!qIsNaN(m_weatherData->windSpeed) && static_cast<int>(m_weatherData->windSpeed) == 0) {
        lastObservation.setWindDirection(QStringLiteral("VR")); // Variable/calm
    } else if (!m_weatherData->windDirection.isEmpty()) {
        lastObservation.setWindDirection(m_weatherData->windDirection);
    }

    // Check if UV index is available for the location
    if (!m_weatherData->UVIndex.isEmpty()) {
        lastObservation.setUVIndex(m_weatherData->UVIndex.toInt());
    }

    forecast->setLastObservation(lastObservation);

    CurrentDay currentDay;

    if (!qIsNaN(m_weatherData->normalHigh)) {
        currentDay.setNormalHighTemp(m_weatherData->normalHigh);
    }
    if (!qIsNaN(m_weatherData->normalLow)) {
        currentDay.setNormalLowTemp(m_weatherData->normalLow);
    }
    // records
    if (!qIsNaN(m_weatherData->recordHigh)) {
        currentDay.setRecordHighTemp(m_weatherData->recordHigh);
    }
    if (!qIsNaN(m_weatherData->recordLow)) {
        currentDay.setRecordLowTemp(m_weatherData->recordLow);
    }
    if (!qIsNaN(m_weatherData->recordRain)) {
        currentDay.setRecordRainfall(m_weatherData->recordRain);
    }
    if (!qIsNaN(m_weatherData->recordSnow)) {
        currentDay.setRecordSnowfall(m_weatherData->recordSnow);
    }

    forecast->setCurrentDay(currentDay);

    auto returnWarnings = std::make_shared<Warnings>();

    const auto &warnings = m_weatherData->warnings;

    for (int k = 0; k < warnings.size(); ++k) {
        const std::shared_ptr<WeatherData::WeatherEvent> &warning = warnings.at(k);

        Warning returnWarning(warning->priority, warning->description);
        returnWarning.setInfo(warning->url);
        returnWarning.setTimestamp(warning->timestamp);

        returnWarnings->addWarning(returnWarning);
    }

    forecast->setWarnings(returnWarnings);

    auto futureDays = std::make_shared<FutureDays>();

    const auto &forecasts = m_weatherData->forecasts;

    // m_weatherData->forecasts is a list with mixed day and night forecasts. Convert them to
    // FutureDays which have day and night spit. Also update and properly show forecast period.
    for (int i = 0; i < forecasts.size(); ++i) {
        const auto &current = forecasts[i];

        std::shared_ptr<WeatherData::ForecastInfo> next = nullptr;

        if (i + 1 < forecasts.size()) {
            next = forecasts[i + 1];
        }

        // The first two forecasts are named "Today" and "Tonight". It's possible that "Today" is omitted.
        // If the first day is a "Today" forecast add it to a full day forecast
        if (current->forecastPeriod == QStringLiteral("Today")) {
            // if the second day is a "Tonight" forecast add it too
            if (next && next->forecastPeriod == QStringLiteral("Tonight")) {
                FutureDayForecast futureDayForecast(current->forecastTimestamp);
                futureDayForecast.setDaytime(forecastInfoToFutureForecast(current));
                futureDayForecast.setNight(forecastInfoToFutureForecast(next));
                futureDays->addDay(futureDayForecast);
                ++i;
            } else {
                // else just create a full day forecast only with a "Today" forecast
                FutureDayForecast futureDayForecast(current->forecastTimestamp);
                futureDayForecast.setDaytime(forecastInfoToFutureForecast(current));
                futureDays->addDay(futureDayForecast);
            }
            // if the forecast starts with night then create a full day forecast only with "Tonight"
        } else if (current->forecastPeriod == QStringLiteral("Tonight")) {
            FutureDayForecast futureDayForecast(current->forecastTimestamp);
            futureDayForecast.setNight(forecastInfoToFutureForecast(current));
            futureDays->addDay(futureDayForecast);
        } else {
            // Then check for next days. Next days are named according to the week days, but with exception
            // that in the night forecasts added "night" at the end of the name.
            // check if the forecast is a daytime forecast
            if (!current->forecastPeriod.contains(QStringLiteral("night"))) {
                // if yes then check the next forecast for night
                if (next && next->forecastPeriod.contains(QStringLiteral("night")) && next->forecastPeriod.contains(current->forecastPeriod)) {
                    // if the next forecast is a night forecast then create full day forecast with day and night forecasts accordingly
                    FutureDayForecast futureDayForecast(current->forecastTimestamp);
                    futureDayForecast.setDaytime(forecastInfoToFutureForecast(current));
                    futureDayForecast.setNight(forecastInfoToFutureForecast(next));
                    futureDays->addDay(futureDayForecast);
                    ++i;
                } else {
                    // if the next forecast is not a night forecast then add only the daytime forecast
                    FutureDayForecast futureDayForecast(current->forecastTimestamp);
                    futureDayForecast.setDaytime(forecastInfoToFutureForecast(current));
                    futureDays->addDay(futureDayForecast);
                }
            } else {
                // if no then create a full day forecast just with the night forecast
                FutureDayForecast futureDayForecast(current->forecastTimestamp);
                futureDayForecast.setNight(forecastInfoToFutureForecast(current));
                futureDays->addDay(futureDayForecast);
            }
        }
    }

    qCDebug(WEATHER::ION::ENVCAN) << "Forecasts added. Total: " << futureDays->columnCount();

    forecast->setFutureDays(futureDays);

    LastDay lastDay;

    // yesterday
    if (!qIsNaN(m_weatherData->prevHigh)) {
        lastDay.setNormalHighTemp(m_weatherData->prevHigh);
    }
    if (!qIsNaN(m_weatherData->prevLow)) {
        lastDay.setNormalLowTemp(m_weatherData->prevLow);
    }
    if (!m_weatherData->prevPrecipType.isEmpty()) {
        lastDay.setPrecipTotal(m_weatherData->prevPrecipTotal);
    }

    forecast->setLastDay(lastDay);

    m_forecastPromise->addResult(forecast);

    qCDebug(WEATHER::ION::ENVCAN) << "Updated weather for: " << m_territoryName << ", " << m_cityCode;
}

#include "ion_envcan.moc"
#include "moc_ion_envcan.cpp"
