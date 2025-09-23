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

QMap<QString, Ion::ConditionIcons> DWDIon::getUniversalIcons() const
{
    return QMap<QString, ConditionIcons>{
        {u"4"_s, Overcast},      {u"5"_s, Mist},          {u"6"_s, Mist},         {u"7"_s, LightRain}, {u"8"_s, Rain},          {u"9"_s, Rain},
        {u"10"_s, LightRain},    {u"11"_s, Rain},         {u"12"_s, Flurries},    {u"13"_s, RainSnow}, {u"14"_s, LightSnow},    {u"15"_s, Snow},
        {u"16"_s, Snow},         {u"17"_s, Hail},         {u"18"_s, LightRain},   {u"19"_s, Rain},     {u"20"_s, Flurries},     {u"21"_s, RainSnow},
        {u"22"_s, LightSnow},    {u"23"_s, Snow},         {u"24"_s, Hail},        {u"25"_s, Hail},     {u"26"_s, Thunderstorm}, {u"27"_s, Thunderstorm},
        {u"28"_s, Thunderstorm}, {u"29"_s, Thunderstorm}, {u"30"_s, Thunderstorm}};
}

QMap<QString, Ion::ConditionIcons> DWDIon::setupDayIconMappings() const
{
    QMap<QString, ConditionIcons> universalIcons = getUniversalIcons();
    QMap<QString, ConditionIcons> dayIcons = {{u"1"_s, ClearDay}, {u"2"_s, FewCloudsDay}, {u"3"_s, PartlyCloudyDay}, {u"31"_s, ClearWindyDay}};
    dayIcons.insert(universalIcons);
    return dayIcons;
}

QMap<QString, Ion::ConditionIcons> DWDIon::setupNightIconMappings() const
{
    QMap<QString, ConditionIcons> universalIcons = getUniversalIcons();
    QMap<QString, ConditionIcons> nightIcons = {{u"1"_s, ClearNight}, {u"2"_s, FewCloudsNight}, {u"3"_s, PartlyCloudyNight}, {u"31"_s, ClearWindyNight}};
    nightIcons.insert(universalIcons);
    return nightIcons;
}

QMap<QString, Ion::WindDirections> DWDIon::setupWindIconMappings() const
{
    return QMap<QString, WindDirections>{
        {u"0"_s, N},   {u"10"_s, N},    {u"20"_s, NNE},  {u"30"_s, NNE},  {u"40"_s, NE},  {u"50"_s, NE},   {u"60"_s, ENE},  {u"70"_s, ENE},  {u"80"_s, E},
        {u"90"_s, E},  {u"100"_s, E},   {u"120"_s, ESE}, {u"130"_s, ESE}, {u"140"_s, SE}, {u"150"_s, SE},  {u"160"_s, SSE}, {u"170"_s, SSE}, {u"180"_s, S},
        {u"190"_s, S}, {u"200"_s, SSW}, {u"210"_s, SSW}, {u"220"_s, SW},  {u"230"_s, SW}, {u"240"_s, WSW}, {u"250"_s, WSW}, {u"260"_s, W},   {u"270"_s, W},
        {u"280"_s, W}, {u"290"_s, WNW}, {u"300"_s, WNW}, {u"310"_s, NW},  {u"320"_s, NW}, {u"330"_s, NNW}, {u"340"_s, NNW}, {u"350"_s, N},   {u"360"_s, N},
    };
}

QMap<QString, Ion::ConditionIcons> const &DWDIon::dayIcons() const
{
    static QMap<QString, ConditionIcons> const dval = setupDayIconMappings();
    return dval;
}

QMap<QString, Ion::ConditionIcons> const &DWDIon::nightIcons() const
{
    static QMap<QString, ConditionIcons> const dval = setupNightIconMappings();
    return dval;
}

QMap<QString, Ion::WindDirections> const &DWDIon::windIcons() const
{
    static QMap<QString, WindDirections> const wval = setupWindIconMappings();
    return wval;
}

// KJob *DWDIon::requestAPIJob(const QString &source, const QUrl &url)
KJob *DWDIon::requestAPIJob(const QUrl &url, QByteArray &result)
{
    KIO::TransferJob *getJob = KIO::get(url, KIO::Reload, KIO::HideProgressInfo);
    getJob->addMetaData(u"cookies"_s, u"none"_s);

    qCDebug(WEATHER::ION::DWD) << "Requesting URL:" << url;

    connect(getJob, &KIO::TransferJob::data, this, [this, &result](KIO::Job *job, const QByteArray &data) {
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
    m_weatherData.reset();
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

    if (info.size() == 4 && info[1] == "weather" && !m_isLegacy) {
        m_isLegacy = true;
        fetchForecast(m_forecastPromise, info[2] + '|' + info[3]);
        return;
    }

    if (info.count() != 2) {
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    m_weatherName = info.first();
    m_weatherID = info.last();

    m_weatherData = std::make_shared<WeatherData>();

    // Fetch forecast data
    const auto getJob = requestAPIJob(QUrl(FORECAST_URL.arg(m_weatherID)), m_forecastData);
    connect(getJob, &KJob::result, this, &DWDIon::forecast_slotJobFinished);
    m_weatherData->isForecastsDataPending = true;

    // Fetch current measurements (different url AND different API, AMAZING)
    const auto getMeasureJob = requestAPIJob(QUrl(MEASURE_URL.arg(m_weatherID)), m_measurementData);
    connect(getMeasureJob, &KJob::result, this, &DWDIon::measure_slotJobFinished);
    m_weatherData->isMeasureDataPending = true;
}

void DWDIon::measure_slotJobFinished(KJob *job)
{
    m_weatherData->isMeasureDataPending = false;

    if (m_forecastPromise->isCanceled()) {
        if (m_weatherData->isForecastsDataPending) {
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

    if (!m_weatherData->isForecastsDataPending) {
        updateWeather();
    }
}

void DWDIon::forecast_slotJobFinished(KJob *job)
{
    m_weatherData->isForecastsDataPending = false;

    if (m_forecastPromise->isCanceled()) {
        if (!m_weatherData->isMeasureDataPending) {
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

    if (!m_weatherData->isMeasureDataPending) {
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
        if (name.contains(flatSearchText, Qt::CaseInsensitive)) {
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
        // Forecast data
        QVariantList daysList = weatherMap[u"days"_s].toList();

        QList<WeatherData::ForecastInfo> &forecasts = m_weatherData->forecasts;

        WeatherData &weatherData = *m_weatherData;

        // Flush out the old forecasts when updating.
        forecasts.clear();

        int dayNumber = 0;

        for (const QVariant &day : daysList) {
            QMap dayMap = day.toMap();
            QString period = dayMap[u"dayDate"_s].toString();
            QString cond = dayMap[u"icon"_s].toString();

            WeatherData::ForecastInfo forecast;
            forecast.period = QDateTime::fromString(period, u"yyyy-MM-dd"_s);
            forecast.tempHigh = parseNumber(dayMap[u"temperatureMax"_s]);
            forecast.tempLow = parseNumber(dayMap[u"temperatureMin"_s]);
            forecast.precipitation = dayMap[u"precipitation"_s].toInt();
            forecast.iconName = getWeatherIcon(dayIcons(), cond);

            if (dayNumber == 0) {
                // These alternative measurements are used, when the stations doesn't have it's own measurements, uses forecast data from the current day
                weatherData.windSpeedAlt = parseNumber(dayMap[u"windSpeed"_s]);
                weatherData.gustSpeedAlt = parseNumber(dayMap[u"windGust"_s]);
                QString windDirection = roundWindDirections(dayMap[u"windDirection"_s].toInt());
                weatherData.windDirectionAlt = getWindDirectionIcon(windIcons(), windDirection);

                // Also fetch today's sunrise and sunset times to determine whether to pick day or night icons
                weatherData.sunriseTime = parseDateFromMSecs(dayMap[u"sunrise"_s].toLongLong());
                weatherData.sunsetTime = parseDateFromMSecs(dayMap[u"sunset"_s].toLongLong());
            }

            forecasts.append(forecast);

            dayNumber++;
            // Only get the next 7 days (including today)
            if (dayNumber == 7) {
                break;
            }
        }

        // Warnings data
        QVariantList warningData = weatherMap[u"warnings"_s].toList();

        QList<WeatherData::WarningInfo> &warningList = weatherData.warnings;

        // Flush out the old forecasts when updating.
        warningList.clear();

        for (const QVariant &warningElement : warningData) {
            QMap warningMap = warningElement.toMap();

            WeatherData::WarningInfo warning;
            warning.headline = warningMap[u"headline"_s].toString();
            warning.description = warningMap[u"description"_s].toString();
            warning.priority = static_cast<Warnings::PriorityClass>(warningMap[u"level"_s].toInt());
            warning.type = warningMap[u"event"_s].toString();
            warning.timestamp = QDateTime::fromMSecsSinceEpoch(warningMap[u"start"_s].toLongLong());

            warningList.append(warning);
        }
    }
}

void DWDIon::parseMeasureData(const QJsonDocument &doc)
{
    WeatherData &weatherData = *m_weatherData;
    QVariantMap weatherMap = doc.object().toVariantMap();

    if (!weatherMap.isEmpty()) {
        weatherData.observationDateTime = parseDateFromMSecs(weatherMap[u"time"_s]);

        weatherData.condIconNumber = weatherMap[u"icon"_s].toString();

        bool windIconValid = false;
        const int windDirection = weatherMap[u"winddirection"_s].toInt(&windIconValid);
        if (windIconValid) {
            weatherData.windDirection = getWindDirectionIcon(windIcons(), roundWindDirections(windDirection));
        }

        weatherData.temperature = parseNumber(weatherMap[u"temperature"_s]);
        weatherData.humidity = parseNumber(weatherMap[u"humidity"_s]);
        weatherData.pressure = parseNumber(weatherMap[u"pressure"_s]);
        weatherData.windSpeed = parseNumber(weatherMap[u"meanwind"_s]);
        weatherData.gustSpeed = parseNumber(weatherMap[u"maxwind"_s]);
        weatherData.dewpoint = parseNumber(weatherMap[u"dewpoint"_s]);
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

    auto forecast = std::make_shared<Forecast>();

    Station station;

    station.setPlace(m_weatherName);
    station.setStation(m_weatherName);

    if (m_isLegacy) {
        station.setNewPlaceInfo(m_weatherName + u"|"_s + m_weatherID);
    }

    forecast->setStation(station);

    MetaData metaData;

    metaData.setCredit(i18nc("credit line, don't change name!", "Source: Deutscher Wetterdienst"));
    metaData.setCreditURL(u"https://www.dwd.de/"_s);
    metaData.setTemperatureUnit(KUnitConversion::Celsius);
    metaData.setWindSpeedUnit(KUnitConversion::KilometerPerHour);
    metaData.setHumidityUnit(KUnitConversion::Percent);
    metaData.setPressureUnit(KUnitConversion::Hectopascal);

    forecast->setMetadata(metaData);

    LastObservation lastObservation;

    if (!m_weatherData->observationDateTime.isNull()) {
        lastObservation.setObservationTimestamp(m_weatherData->observationDateTime);
    } else {
        lastObservation.setObservationTimestamp(QDateTime::currentDateTime());
    }

    if (!m_weatherData->condIconNumber.isEmpty()) {
        lastObservation.setConditionIcon(getWeatherIcon(isNightTime(*m_weatherData) ? nightIcons() : dayIcons(), m_weatherData->condIconNumber));
    }

    if (!qIsNaN(m_weatherData->temperature)) {
        lastObservation.setTemperature(m_weatherData->temperature);
    }
    if (!qIsNaN(m_weatherData->humidity)) {
        lastObservation.setHumidity(m_weatherData->humidity);
    }
    if (!qIsNaN(m_weatherData->pressure)) {
        lastObservation.setPressure(m_weatherData->pressure);
    }
    if (!qIsNaN(m_weatherData->dewpoint)) {
        lastObservation.setDewpoint(m_weatherData->dewpoint);
    }

    if (!qIsNaN(m_weatherData->windSpeed)) {
        lastObservation.setWindSpeed(m_weatherData->windSpeed);
    } else {
        lastObservation.setWindSpeed(m_weatherData->windSpeedAlt);
    }

    if (!qIsNaN(m_weatherData->gustSpeed)) {
        lastObservation.setWindGust(m_weatherData->gustSpeed);
    } else {
        lastObservation.setWindGust(m_weatherData->gustSpeedAlt);
    }

    if (!m_weatherData->windDirection.isEmpty()) {
        lastObservation.setWindDirection(m_weatherData->windDirection);
    } else {
        lastObservation.setWindDirection(m_weatherData->windDirectionAlt);
    }

    forecast->setLastObservation(lastObservation);

    if (!m_weatherData->forecasts.isEmpty()) {
        auto futureDays = std::make_shared<FutureDays>();

        int days = 0;
        for (const auto &dayForecast : m_weatherData->forecasts) {
            FutureDayForecast futureDay;
            QString period;
            if (days == 0) {
                period = i18nc("Short for Today", "Today");
            } else {
                period = dayForecast.period.toString(u"dddd"_s);

                period.replace(u"Saturday"_s, i18nc("Short for Saturday", "Sat"));
                period.replace(u"Sunday"_s, i18nc("Short for Sunday", "Sun"));
                period.replace(u"Monday"_s, i18nc("Short for Monday", "Mon"));
                period.replace(u"Tuesday"_s, i18nc("Short for Tuesday", "Tue"));
                period.replace(u"Wednesday"_s, i18nc("Short for Wednesday", "Wed"));
                period.replace(u"Thursday"_s, i18nc("Short for Thursday", "Thu"));
                period.replace(u"Friday"_s, i18nc("Short for Friday", "Fri"));
            }

            FutureForecast futureForecast;

            futureForecast.setConditionIcon(dayForecast.iconName);
            futureForecast.setHighTemp(dayForecast.tempHigh);
            futureForecast.setLowTemp(dayForecast.tempLow);

            futureDay.setWeekDay(period);
            futureDay.setDaytime(futureForecast);

            futureDays->addDay(futureDay);
            ++days;
        }
        qCDebug(WEATHER::ION::DWD) << "Total forecast days: " << days;

        forecast->setFutureDays(futureDays);
    }

    if (!m_weatherData->warnings.isEmpty()) {
        auto warnings = std::make_shared<Warnings>();

        for (const auto &localWarning : m_weatherData->warnings) {
            Warning warning(localWarning.priority, u"<p><b>%1</b></p>%2"_s.arg(localWarning.headline, localWarning.description));
            warning.setTimestamp(localWarning.timestamp.toString(u"dd.MM.yyyy"_s));
            warnings->addWarning(warning);
        }
        forecast->setWarnings(warnings);
    }

    qCDebug(WEATHER::ION::DWD) << "Forecast received. Return";

    m_forecastPromise->addResult(forecast);
    m_forecastPromise->finish();
    clearForecastData();
}

/*
 * Helper methods
 */
float DWDIon::parseNumber(const QVariant &number) const
{
    bool isValid = false;
    const int intValue = number.toInt(&isValid);
    if (!isValid) {
        return NAN;
    }
    if (intValue == 0x7fff) { // DWD uses 32767 to mark an error value
        return NAN;
    }
    // e.g. DWD API int 17 equals 1.7
    return static_cast<float>(intValue) / 10;
}

QDateTime DWDIon::parseDateFromMSecs(const QVariant &timestamp) const
{
    bool isValid = false;
    const qint64 msecs = timestamp.toLongLong(&isValid);

    return isValid ? QDateTime::fromMSecsSinceEpoch(msecs) : QDateTime();
}

QString DWDIon::roundWindDirections(int windDirection) const
{
    QString roundedWindDirection = QString::number(qRound(((float)windDirection) / 100) * 10);
    return roundedWindDirection;
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

bool DWDIon::isNightTime(const WeatherData &weatherData) const
{
    if (weatherData.sunriseTime.isNull() || weatherData.sunsetTime.isNull()) {
        // default to daytime icons if we're missing sunrise/sunset times
        return false;
    }

    return weatherData.observationDateTime < weatherData.sunriseTime || weatherData.observationDateTime > weatherData.sunsetTime;
}

#include "ion_dwd.moc"
#include "moc_ion_dwd.cpp"
