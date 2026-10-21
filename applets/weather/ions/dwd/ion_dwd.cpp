/*
    SPDX-FileCopyrightText: 2021 Emily Ehlert
    SPDX-FileCopyrightText: 2024 Ismael Asensio <isma.af@gmail.com>

    Based upon BBC Weather Ion and ENV Canada Ion by Shawn Starr
    SPDX-FileCopyrightText: 2007-2009 Shawn Starr <shawn.starr@rogers.com>

    also

    the wetter.com Ion by Thilo-Alexander Ginkel
    SPDX-FileCopyrightText: 2009 Thilo-Alexander Ginkel <thilo@ginkel.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/* Ion for weather data from Deutscher Wetterdienst (DWD) / German Weather Service */

#include "ion_dwd.h"

#include "dwd_debug.h"

#include <KIO/TransferJob>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KUnitConversion/Converter>

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QVariant>

K_PLUGIN_CLASS_WITH_JSON(DWDIon, "metadata.json")

using namespace Qt::StringLiterals;

constexpr QLatin1String CATALOGUE_URL = "https://www.dwd.de/DE/leistungen/met_verfahren_mosmix/mosmix_stationskatalog.cfg?view=nasPublication&nn=16102"_L1;
constexpr QLatin1String FORECAST_URL = "https://app-prod-ws.warnwetter.de/v30/stationOverviewExtended?stationIds=%1"_L1;
constexpr QLatin1String MEASURE_URL = "https://s3.eu-central-1.amazonaws.com/app-prod-static.warnwetter.de/v16/current_measurement_%1.json"_L1;

DWDIon::DWDIon(QObject *parent, const QVariantList &args)
    : Ion(parent)
{
    Q_UNUSED(args);
}

DWDIon::~DWDIon()
{
}

QMap<uint, Ion::ConditionIcons> DWDIon::getUniversalIcons() const
{
    return {{4, Overcast},   {5, Mist},          {6, Mist},          {7, LightRain},     {8, Rain},          {9, Rain},         {10, LightRain},
            {11, Rain},      {12, Flurries},     {13, RainSnow},     {14, LightSnow},    {15, Snow},         {16, Snow},        {17, Hail},
            {18, LightRain}, {19, Rain},         {20, Flurries},     {21, RainSnow},     {22, LightSnow},    {23, Snow},        {24, Hail},
            {25, Hail},      {26, Thunderstorm}, {27, Thunderstorm}, {28, Thunderstorm}, {29, Thunderstorm}, {30, Thunderstorm}};
}

QMap<uint, Ion::ConditionIcons> DWDIon::setupDayIconMappings() const
{
    QMap<uint, ConditionIcons> universalIcons = getUniversalIcons();
    QMap<uint, ConditionIcons> dayIcons = {{1, ClearDay}, {2, FewCloudsDay}, {3, PartlyCloudyDay}, {31, ClearWindyDay}};
    dayIcons.insert(universalIcons);
    return dayIcons;
}

QMap<uint, Ion::ConditionIcons> DWDIon::setupNightIconMappings() const
{
    QMap<uint, ConditionIcons> universalIcons = getUniversalIcons();
    QMap<uint, ConditionIcons> nightIcons = {{1, ClearNight}, {2, FewCloudsNight}, {3, PartlyCloudyNight}, {31, ClearWindyNight}};
    nightIcons.insert(universalIcons);
    return nightIcons;
}

QMap<uint, Ion::WindDirections> DWDIon::setupWindIconMappings() const
{
    return {
        {0, N},     {10, N},   {20, NNE}, {30, NNE},  {40, NE},   {50, NE},   {60, ENE}, {70, ENE},  {80, E},    {90, E},    {100, E},  {120, ESE},
        {130, ESE}, {140, SE}, {150, SE}, {160, SSE}, {170, SSE}, {180, S},   {190, S},  {200, SSW}, {210, SSW}, {220, SW},  {230, SW}, {240, WSW},
        {250, WSW}, {260, W},  {270, W},  {280, W},   {290, WNW}, {300, WNW}, {310, NW}, {320, NW},  {330, NNW}, {340, NNW}, {350, N},  {360, N},
    };
}

QMap<uint, Ion::ConditionIcons> const &DWDIon::dayIcons() const
{
    static QMap<uint, ConditionIcons> const dval = setupDayIconMappings();
    return dval;
}

QMap<uint, Ion::ConditionIcons> const &DWDIon::nightIcons() const
{
    static QMap<uint, ConditionIcons> const dval = setupNightIconMappings();
    return dval;
}

QMap<uint, Ion::WindDirections> const &DWDIon::windIcons() const
{
    static QMap<uint, WindDirections> const wval = setupWindIconMappings();
    return wval;
}

// KJob *DWDIon::requestAPIJob(const QString &source, const QUrl &url)
KJob *DWDIon::requestAPIJob(const QUrl &url, QByteArray &result)
{
    KIO::TransferJob *getJob = KIO::get(url, KIO::Reload, KIO::HideProgressInfo);
    getJob->addMetaData(u"cookies"_s, u"none"_s);

    qCDebug(WEATHER::ION::DWD) << "Requesting URL:" << url;

    connect(getJob, &KIO::TransferJob::data, this, [&result](KIO::Job *job, const QByteArray &data) {
        Q_UNUSED(job);
        if (data.isEmpty()) {
            return;
        }
        result.append(data);
    });

    return getJob;
}

void DWDIon::clearForecastData()
{
    m_isLegacy = false;
    m_weatherName.clear();
    m_weatherID.clear();
    m_forecastData.clear();
    m_measurementData.clear();
    m_sunriseTime = {};
    m_sunsetTime = {};
    m_forecast.reset();
    m_forecastPromise.reset();
}

void DWDIon::clearLocationData()
{
    m_locationPromise.reset();
    m_locationsData.clear();
}

void DWDIon::findPlaces(std::shared_ptr<QPromise<std::shared_ptr<Locations>>> promise, const QString &searchString)
{
    m_locationPromise = promise;

    m_locationPromise->start();

    if (m_locationPromise->isCanceled()) {
        qCDebug(WEATHER::ION::DWD) << "Location promise canceled. Return";
        m_locationPromise->finish();
        clearLocationData();
        return;
    }

    m_searchString = searchString;

    // Checks if the stations have already been loaded, always contains the currently active one
    if (!m_place.isEmpty()) {
        qCDebug(WEATHER::ION::DWD) << "place list is not empty. Search in it";
        searchInStationList(m_searchString);
        m_locationPromise->finish();
        clearLocationData();
    } else {
        qCDebug(WEATHER::ION::DWD) << "place list is not empty. Update it first";
        const auto getJob = requestAPIJob(QUrl(CATALOGUE_URL), m_locationsData);
        connect(getJob, &KJob::result, this, &DWDIon::places_slotJobFinished);
    }
}

void DWDIon::places_slotJobFinished(KJob *job)
{
    if (job->error()) {
        qCWarning(WEATHER::ION::DWD) << "error during setup" << job->errorText();
        return;
    }

    if (!m_locationsData.isEmpty()) {
        parseStationData(m_locationsData);
        searchInStationList(m_searchString);
    }

    m_locationPromise->finish();
    clearLocationData();
}

void DWDIon::fetchForecast(std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> promise, const QString &placeInfo)
{
    m_forecastPromise = promise;

    m_forecastPromise->start();

    if (m_forecastPromise->isCanceled()) {
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

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

    m_weatherName = info.first();
    m_weatherID = info.last();

    m_forecast = std::make_shared<Forecast>();

    // Fetch forecast data
    const auto getJob = requestAPIJob(QUrl(FORECAST_URL.arg(m_weatherID)), m_forecastData);
    connect(getJob, &KJob::result, this, &DWDIon::forecast_slotJobFinished);
    m_isForecastsDataPending = true;

    // Fetch current measurements (different url AND different API, AMAZING)
    const auto getMeasureJob = requestAPIJob(QUrl(MEASURE_URL.arg(m_weatherID)), m_measurementData);
    connect(getMeasureJob, &KJob::result, this, &DWDIon::measure_slotJobFinished);
    m_isMeasureDataPending = true;
}

void DWDIon::measure_slotJobFinished(KJob *job)
{
    m_isMeasureDataPending = false;

    if (m_forecastPromise->isCanceled()) {
        if (!m_isForecastsDataPending) {
            m_forecastPromise->finish();
            clearForecastData();
        }
        return;
    }

    if (job->error() || m_measurementData.isEmpty()) {
        qCWarning(WEATHER::ION::DWD) << "no measurements received" << job->errorText();
    } else {
        QJsonDocument doc = QJsonDocument::fromJson(m_measurementData);
        parseMeasureData(doc);
        m_measurementData.clear();
    }

    if (!m_isForecastsDataPending) {
        updateWeather();
    }
}

void DWDIon::forecast_slotJobFinished(KJob *job)
{
    m_isForecastsDataPending = false;

    if (m_forecastPromise->isCanceled()) {
        if (!m_isMeasureDataPending) {
            m_forecastPromise->finish();
            clearForecastData();
        }
        return;
    }

    if (job->error() || m_forecastData.isEmpty()) {
        qCWarning(WEATHER::ION::DWD) << "error during forecast" << job->errorText();
    } else {
        QJsonDocument doc = QJsonDocument::fromJson(m_forecastData);
        parseForecastData(doc);
        m_forecastData.clear();
    }

    if (!m_isMeasureDataPending) {
        updateWeather();
    }
}

void DWDIon::parseStationData(const QByteArray &data)
{
    const QString stringData = QString::fromLatin1(data);
    const QList<QStringView> lines = QStringView(stringData).split(QChar::LineFeed);

    // This loop parses the station file (https://www.dwd.de/DE/leistungen/met_verfahren_mosmix/mosmix_stationskatalog.cfg)
    // ID    ICAO NAME                 LAT    LON     ELEV
    // ----- ---- -------------------- -----  ------- -----
    // 01001 ENJA JAN MAYEN             70.56   -8.40    10
    // 01008 ENSB SVALBARD              78.15   15.28    29

    // Extract the fields' names and positions from the header
    QMap<QString, std::pair<int, int>> fields;
    const auto &titles = lines[0];
    const auto delimiters = lines[1].split(QChar::Space);

    int position = 0;
    for (const QStringView &delimiter : delimiters) {
        const int length = qMin(delimiter.length(), titles.length() - position);
        const QString fieldName = titles.sliced(position, length).trimmed().toString().toLower();
        fields.insert(fieldName, {position, length});
        position += length + 1;
    }

    if (!fields.contains(u"id"_s) || !fields.contains(u"name"_s)) {
        qCWarning(WEATHER::ION::DWD) << "Error parsing station list. Cannot recognize header";
        return;
    }

    auto extractField = [&fields](QStringView line, const QString &fieldName) {
        const auto &[pos, length] = fields[fieldName];
        if (pos + length > line.length()) [[unlikely]] {
            return line.sliced(pos).trimmed();
        }
        return line.sliced(pos, length).trimmed();
    };

    for (int idx = 2; idx < lines.count(); idx++) {
        const QStringView &line = lines.at(idx);

        const QString id = extractField(line, u"id"_s).toString();
        // This checks if this station is a station we know is working
        // With this we remove all non working but also a lot of working ones
        if (!id.startsWith('0'_L1) && !id.startsWith('1'_L1)) {
            break;
        }
        const QString name = extractField(line, u"name"_s).toString();
        m_place.insert(camelCaseString(name), id);
    }
    qCDebug(WEATHER::ION::DWD) << "Number of parsed stations: " << m_place.size();
}

void DWDIon::searchInStationList(const QString &searchText)
{
    QString flatSearchText = searchText;
    flatSearchText // The station list does not contains umlauts
        .replace(u"ä"_s, u"ae"_s)
        .replace(u"ö"_s, u"oe"_s)
        .replace(u"ü"_s, u"ue"_s)
        .replace(u"ß"_s, u"ss"_s);

    qCDebug(WEATHER::ION::DWD) << "Searching in station list:" << flatSearchText;

    for (const auto [name, id] : m_place.asKeyValueRange()) {
        if (matchesSearchString(name, flatSearchText)) {
            m_locations.append(name);
        }
    }

    validate();
}

void DWDIon::parseForecastData(const QJsonDocument &doc)
{
    QVariantMap weatherMap = doc.object().toVariantMap();
    if (weatherMap.isEmpty()) {
        qCDebug(WEATHER::ION::DWD) << "Forecast data is empty";
        return;
    }
    weatherMap = weatherMap.first().toMap(); // Mind the .first(). It needs guarding against isEmpty.
    if (!weatherMap.isEmpty()) {
        // Hourly Forecast data
        const QVariantMap forecast1Map = weatherMap[u"forecast1"_s].toMap();

        const qlonglong start = forecast1Map[u"start"_s].toLongLong();
        const qlonglong timeStep = forecast1Map[u"timeStep"_s].toLongLong();

        const QVariantList tempList = forecast1Map[u"temperature"_s].toList();
        const QVariantList precipList = forecast1Map[u"precipitationTotal"_s].toList();
        const QVariantList iconList = forecast1Map[u"icon1h"_s].toList();
        const QVariantList isDayList = forecast1Map[u"isDay"_s].toList();

        const qsizetype count = std::min(tempList.size(), iconList.size());

        const auto futureHours = std::make_shared<FutureHours>();

        for (qsizetype hourNumber = 0; hourNumber < count; ++hourNumber) {
            const qlonglong timestamp = start + (static_cast<qlonglong>(hourNumber) * timeStep);

            // DWD provides forecasts starting from the beginning of the day. However, for most stations,
            // forecast data for hours that have already passed is no longer available from the server.
            //
            // Skip entries with timestamps in the past. Otherwise, the forecast would start with missing
            // data in most cases.
            if (timestamp <= QDateTime::currentMSecsSinceEpoch()) {
                continue;
            }

            FutureHourForecast futureForecast(QDateTime::fromMSecsSinceEpoch(timestamp));

            if (hourNumber < tempList.size()) {
                const auto temp = parseNumber(tempList[hourNumber]);
                if (temp.has_value()) {
                    futureForecast.setGeneralTemp(temp.value());
                }
            }

            if (hourNumber < precipList.size()) {
                const auto precipitation = parseNumber(precipList[hourNumber]);
                if (precipitation.has_value()) {
                    futureForecast.setConditionProbability(precipitation.value());
                }
            }

            if (hourNumber < iconList.size()) {
                bool isDay = true;
                if (hourNumber < isDayList.size()) {
                    isDay = isDayList[hourNumber].toBool();
                }
                bool iconCodeValid = false;
                int iconCode = iconList.value(hourNumber).toInt(&iconCodeValid);
                if (iconCodeValid) {
                    if (isDay) {
                        futureForecast.setConditionIcon(getWeatherIcon(dayIcons().value(iconCode)));
                    } else {
                        futureForecast.setConditionIcon(getWeatherIcon(nightIcons().value(iconCode)));
                    }
                }
            }

            futureHours->addHour(futureForecast);
        }

        m_forecast->setFutureHours(futureHours);

        // Day Forecast data
        QVariantList daysList = weatherMap[u"days"_s].toList();

        auto futureDays = std::make_shared<FutureDays>();

        int dayNumber = 0;

        for (const QVariant &day : daysList) {
            QMap dayMap = day.toMap();
            QString period = dayMap[u"dayDate"_s].toString();

            FutureDayForecast futureDayForecast(QDateTime::fromString(period, u"yyyy-MM-dd"_s));

            FutureForecast futureForecast;

            const auto tempHigh = parseNumber(dayMap[u"temperatureMax"_s]);
            if (tempHigh.has_value()) {
                futureForecast.setHighTemp(tempHigh.value());
            }

            const auto tempLow = parseNumber(dayMap[u"temperatureMin"_s]);
            if (tempLow.has_value()) {
                futureForecast.setLowTemp(tempLow.value());
            }

            bool precipitationValid = false;
            const int precipitation = dayMap[u"precipitation"_s].toInt(&precipitationValid);
            if (precipitationValid) {
                futureForecast.setConditionProbability(precipitation);
            }

            bool iconNameValid = false;
            const auto iconName = (dayMap[u"icon"_s].toInt(&iconNameValid));
            if (iconNameValid) {
                futureForecast.setConditionIcon(getWeatherIcon(dayIcons().value(iconName)));
            }

            if (dayNumber == 0) {
                // Also fetch today's sunrise and sunset times to determine whether to pick day or night icons
                const auto sunriseTime = parseDateFromMSecs(dayMap[u"sunrise"_s]);
                if (sunriseTime.has_value()) {
                    m_sunriseTime = sunriseTime.value();
                }
                const auto sunsetTime = parseDateFromMSecs(dayMap[u"sunset"_s]);
                if (sunsetTime.has_value()) {
                    m_sunsetTime = sunsetTime.value();
                }
            }

            futureDayForecast.setDaytime(futureForecast);
            futureDays->addDay(futureDayForecast);

            dayNumber++;
            // Only get the next 7 days (including today)
            if (dayNumber == 7) {
                break;
            }
        }

        m_forecast->setFutureDays(futureDays);

        // Warnings data
        QVariantList warningData = weatherMap[u"warnings"_s].toList();

        auto warnings = std::make_shared<Warnings>();

        for (const QVariant &warningElement : warningData) {
            QMap warningMap = warningElement.toMap();

            const QString headline = warningMap[u"headline"_s].toString();
            const QString description = warningMap[u"description"_s].toString();
            if (headline.isEmpty() && description.isEmpty()) {
                continue;
            }

            bool priorityValid = false;
            auto priority = warningMap[u"level"_s].toInt(&priorityValid);
            if (!priorityValid) {
                priority = Warnings::Low;
            }

            Warning warning(static_cast<Warnings::PriorityClass>(priority), u"<p><b>%1</b></p>%2"_s.arg(headline, description));
            const auto timestamp = parseDateFromMSecs(warningMap[u"start"_s]);
            if (timestamp.has_value()) {
                warning.setTimestamp(timestamp.value());
            }

            warnings->addWarning(warning);
        }

        m_forecast->setWarnings(warnings);
    }
}

void DWDIon::parseMeasureData(const QJsonDocument &doc)
{
    QVariantMap weatherMap = doc.object().toVariantMap();

    if (!weatherMap.isEmpty()) {
        LastObservation lastObservation;
        const auto timestamp = parseDateFromMSecs(weatherMap[u"time"_s]);
        if (timestamp.has_value()) {
            lastObservation.setObservationTimestamp(timestamp.value());
        }

        bool condIconValid = false;
        const int condIconNumber = weatherMap[u"icon"_s].toInt(&condIconValid);
        if (condIconValid) {
            if (timestamp.has_value() && isNightTime(timestamp.value())) {
                lastObservation.setConditionIcon(getWeatherIcon(nightIcons().value(condIconNumber)));
            } else {
                lastObservation.setConditionIcon(getWeatherIcon(dayIcons().value(condIconNumber)));
            }
        }

        bool windIconValid = false;
        const int windDirection = weatherMap[u"winddirection"_s].toInt(&windIconValid);
        if (windIconValid) {
            lastObservation.setWindDirection(getWindDirectionIcon(windIcons().value(roundWindDirections(windDirection))));
        }

        const auto temperature = parseNumber(weatherMap[u"temperature"_s]);
        if (temperature.has_value()) {
            lastObservation.setTemperature(temperature.value());
        }

        const auto humidity = parseNumber(weatherMap[u"humidity"_s]);
        if (humidity.has_value()) {
            lastObservation.setHumidity(humidity.value());
        }

        const auto pressure = parseNumber(weatherMap[u"pressure"_s]);
        if (pressure.has_value()) {
            lastObservation.setPressure(pressure.value());
        }

        const auto windSpeed = parseNumber(weatherMap[u"meanwind"_s]);
        if (windSpeed.has_value()) {
            lastObservation.setWindSpeed(windSpeed.value());
        }

        const auto gustSpeed = parseNumber(weatherMap[u"maxwind"_s]);
        if (gustSpeed.has_value()) {
            lastObservation.setWindGust(gustSpeed.value());
        }

        const auto dewpoint = parseNumber(weatherMap[u"dewpoint"_s]);
        if (dewpoint.has_value()) {
            lastObservation.setDewpoint(dewpoint.value());
        }

        m_forecast->setLastObservation(lastObservation);
    }
}

void DWDIon::validate()
{
    if (m_locations.isEmpty()) {
        qCDebug(WEATHER::ION::DWD) << "No locations found";
        return;
    }

    qCDebug(WEATHER::ION::DWD) << "Validating locations. Total: " << m_locations.size();

    auto locations = std::make_shared<Locations>();

    for (const QString &place : std::as_const(m_locations)) {
        Location location;
        location.setStation(place);
        location.setDisplayName(place);
        location.setCode(m_place[place]);
        location.setPlaceInfo(place + u"|"_s + m_place[place]);
        locations->addLocation(location);
    }

    m_locationPromise->addResult(locations);

    m_locations.clear();
}

void DWDIon::updateWeather()
{
    qCDebug(WEATHER::ION::DWD) << "Updating weather";

    Station station;

    station.setPlace(m_weatherName);
    station.setStation(m_weatherName);

    if (m_isLegacy) {
        station.setNewPlaceInfo(m_weatherName + u"|"_s + m_weatherID);
    }

    m_forecast->setStation(station);

    MetaData metaData;

    metaData.setCredit(i18nc("credit line, don't change name!", "Source: Deutscher Wetterdienst"));
    metaData.setCreditURL(u"https://www.dwd.de/"_s);
    metaData.setTemperatureUnit(KUnitConversion::Celsius);
    metaData.setWindSpeedUnit(KUnitConversion::KilometerPerHour);
    metaData.setHumidityUnit(KUnitConversion::Percent);
    metaData.setPressureUnit(KUnitConversion::Hectopascal);

    m_forecast->setMetadata(metaData);

    qCDebug(WEATHER::ION::DWD) << "Forecast received. Return";

    m_forecastPromise->addResult(m_forecast);
    m_forecastPromise->finish();
    clearForecastData();
}

/*
 * Helper methods
 */
std::optional<qreal> DWDIon::parseNumber(const QVariant &number) const
{
    bool isValid = false;
    const int intValue = number.toInt(&isValid);
    if (!isValid) {
        return std::nullopt;
    }
    if (intValue == 0x7fff) { // DWD uses 32767 to mark an error value
        return std::nullopt;
    }
    // e.g. DWD API int 17 equals 1.7
    return static_cast<qreal>(intValue) / 10;
}

std::optional<QDateTime> DWDIon::parseDateFromMSecs(const QVariant &timestamp) const
{
    bool isValid = false;
    const qint64 msecs = timestamp.toLongLong(&isValid);

    if (!isValid) {
        return std::nullopt;
    }

    return QDateTime::fromMSecsSinceEpoch(msecs);
}

int DWDIon::roundWindDirections(int windDirection) const
{
    return qRound(((float)windDirection) / 100) * 10;
}

QString DWDIon::camelCaseString(const QString &text) const
{
    QString result;
    bool nextBig = true;

    for (QChar c : text) {
        if (c.isLetter()) {
            if (nextBig) {
                result.append(c.toUpper());
                nextBig = false;
            } else {
                result.append(c.toLower());
            }
        } else {
            if (c == QChar::Space || c == '-'_L1) {
                nextBig = true;
            }
            result.append(c);
        }
    }

    return result;
}

bool DWDIon::isNightTime(const QDateTime &dateTime) const
{
    if (m_sunriseTime.isNull() || m_sunsetTime.isNull()) {
        // default to daytime icons if we're missing sunrise/sunset times
        return false;
    }

    return dateTime < m_sunriseTime || dateTime > m_sunsetTime;
}

#include "ion_dwd.moc"
#include "moc_ion_dwd.cpp"
