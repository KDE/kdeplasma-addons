/*
    SPDX-FileCopyrightText: 2007-2009, 2019 Shawn Starr <shawn.starr@rogers.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/* Ion for Environment Canada XML data */

#pragma once

#include "ion.h"

#include <QDateTime>
#include <QXmlStreamReader>

#include <forecast.h>
#include <locations.h>

class KJob;
namespace KIO
{
class Job;
} // namespace KIO

class WeatherData
{
public:
    WeatherData();

    // WeatherEvent can have more than one, especially in Canada, eh? :)
    struct WeatherEvent {
        QString url;
        Warnings::PriorityClass priority;
        QString description;
        QDateTime timestamp;
    };

    struct DayForecastInfo {
        DayForecastInfo();

        QDateTime forecastTimestamp;
        QString forecastPeriod;
        QString forecastSummary;
        QString iconName;
        QString shortForecast;

        float tempHigh;
        float tempLow;
        float popPrecent;
        QString windForecast;

        QString precipForecast;
        QString precipType;
        QString precipTotalExpected;
        int forecastHumidity;
    };

    QList<std::shared_ptr<DayForecastInfo>> dayForecasts;

    struct HourlyForecastInfo {
        HourlyForecastInfo();

        QDateTime forecastTimestamp;
        QString forecastSummary;
        QString iconName;

        float temp;
        float lopPrecent;

        float windSpeed;
        float gust;
        QString windDirection;

        int forecastHumidity;
    };

    QList<std::shared_ptr<HourlyForecastInfo>> hourlyForecasts;

    QString creditUrl;
    QString countryName;
    QString longTerritoryName;
    QString shortTerritoryName;
    QString cityName;
    QString regionName;
    QString stationID;
    double stationLatitude;
    double stationLongitude;

    // Current observation information.
    QString obsTimestamp;
    QDateTime observationDateTime;

    QString condition;
    float temperature;
    float dewpoint;

    // In winter windchill, in summer, humidex
    QString humidex;
    float windchill;

    float pressure;
    QString pressureTendency;

    float visibility;
    float humidity;

    float windSpeed;
    float windGust;
    QString windDirection;
    QString windDegrees;

    QList<std::shared_ptr<WeatherEvent>> warnings;

    float normalHigh;
    float normalLow;

    QDateTime dayForecastTimestamp;
    QDateTime hourlyForecastTimestamp;

    QString UVIndex;
    QString UVRating;

    // Historical data from previous day.
    float prevHigh;
    float prevLow;
    QString prevPrecipType;
    float prevPrecipTotal;

    // Almanac info
    QDateTime sunriseTimestamp;
    QDateTime sunsetTimestamp;
    QDateTime moonriseTimestamp;
    QDateTime moonsetTimestamp;

    // Historical Records
    float recordHigh;
    float recordLow;
    float recordRain;
    float recordSnow;

    bool isNight = false;

    // Fields to get the forecast url
    struct UrlInfo {
        QString province;
        QString cityCode;
        QStringList hours;
        int hourIndex = 0;
        QString fileName;
        int requests = 0;
    };

    UrlInfo urlInfo;
};

/**
 * https://weather.gc.ca/mainmenu/disclaimer_e.html
 */
class EnvCanadaIon : public Ion
{
    Q_OBJECT

public:
    explicit EnvCanadaIon(QObject *parent, const QVariantList &args);
    ~EnvCanadaIon() override;

    void findPlaces(std::shared_ptr<QPromise<std::shared_ptr<Locations>>> promise, const QString &searchString) override;

    void fetchForecast(std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> promise, const QString &placeInfo) override;

private:
    void updateWeather();

    // helper functions used to update forecast days in updateWeather
    FutureForecast forecastInfoToFutureForecast(const std::shared_ptr<WeatherData::DayForecastInfo> &info);
    QDateTime dateTimeForForecastPeriod(const QString &periodName, const QDate &issueDate) const;

    QMap<QString, ConditionIcons> setupConditionIconMappings() const;
    QMap<QString, ConditionIcons> setupDayConditionIconMappings() const;
    QMap<QString, ConditionIcons> setupNightConditionIconMappings() const;
    QMap<QString, ConditionIcons> setupForecastIconMappings() const;
    QMap<QString, ConditionIcons> setupDayForecastIconMappings() const;
    QMap<QString, ConditionIcons> setupNightForecastIconMappings() const;

    QMap<QString, ConditionIcons> const &dayConditionIcons() const;
    QMap<QString, ConditionIcons> const &nightConditionIcons() const;

    QMap<QString, ConditionIcons> const &dayForecastIcons() const;
    QMap<QString, ConditionIcons> const &nightForecastIcons() const;

    // Load and Parse the place XML listing
    void readXMLSetup();

    // Load and parse the specific place(s)
    void readXMLData(QXmlStreamReader &xml);

    // New API on envcan: need to get a specifically named file
    void getWeatherData();
    // Parse a directory listing with files or folders as hyperlinks
    void parseDirListing(WeatherData::UrlInfo &info, QXmlStreamReader &xml);

    // Check if place specified is valid or not
    void validate(const QString &source);

    // Catchall for unknown XML tags
    void parseUnknownElement(QXmlStreamReader &xml) const;

    // Parse weather XML data
    void parseWeatherSite(WeatherData &data, QXmlStreamReader &xml);

    // Locations parsing
    void parseLocations(WeatherData &data, QXmlStreamReader &xml);
    float parseCoordinate(QStringView coord) const;

    // Conditions parsing
    void parseConditions(WeatherData &data, QXmlStreamReader &xml);
    void parseWindInfo(WeatherData &data, QXmlStreamReader &xml);

    // Hourly forecast parsing
    void parseHourlyForecast(WeatherData &data, QXmlStreamReader &xml);

    // Day forecast parsing
    void parseDayForecast(WeatherData &data, QXmlStreamReader &xml);
    void parseRegionalNormals(WeatherData &data, QXmlStreamReader &xml);
    void parseForecast(WeatherData &data, QXmlStreamReader &xml);
    void parseShortForecast(std::shared_ptr<WeatherData::DayForecastInfo> forecast, QXmlStreamReader &xml);
    void parseForecastTemperatures(std::shared_ptr<WeatherData::DayForecastInfo> forecast, QXmlStreamReader &xml);
    void parseWindForecast(std::shared_ptr<WeatherData::DayForecastInfo> forecast, QXmlStreamReader &xml);
    void parsePrecipitationForecast(std::shared_ptr<WeatherData::DayForecastInfo> forecast, QXmlStreamReader &xml);
    void parsePrecipTotals(std::shared_ptr<WeatherData::DayForecastInfo> forecast, QXmlStreamReader &xml);
    void parseUVIndex(WeatherData &data, QXmlStreamReader &xml);

    // Other data parsing
    void parseWarnings(WeatherData &data, QXmlStreamReader &xml);
    void parseYesterdayWeather(WeatherData &data, QXmlStreamReader &xml);
    void parseAstronomicals(WeatherData &data, QXmlStreamReader &xml);
    void parseWeatherRecords(WeatherData &data, QXmlStreamReader &xml);

    // general parsing. Called from different functions
    void parseDateTime(WeatherData &data, QXmlStreamReader &xml, bool isDayForecast = true, std::shared_ptr<WeatherData::WeatherEvent> event = nullptr);
    void parseFloat(float &value, QXmlStreamReader &xml);

    // Clear unneeded data
    void clearForecastData();
    void clearLocationData();

private Q_SLOTS:
    void places_slotDataArrived(KIO::Job *job, const QByteArray &data);
    void places_slotJobFinished(KJob *job);

    void forecast_slotDataArrived(KIO::Job *job, const QByteArray &data);
    void forecast_slotJobFinished(KJob *job);

private:
    struct XMLMapInfo {
        QString cityName;
        QString territoryName;
        QString cityCode;
    };

    // locations data
    QHash<QString, XMLMapInfo> m_places;
    QString m_searchString;
    std::shared_ptr<QXmlStreamReader> m_xmlSetup;

    // Weather information
    std::shared_ptr<WeatherData> m_weatherData;
    QString m_territoryName;
    QString m_cityCode;
    std::shared_ptr<QXmlStreamReader> m_xmlForecast;

    // used to handle the old format of a forecast request string
    bool m_isLegacy = false;
    QString m_legacyPlaceInfo;

    std::shared_ptr<QPromise<std::shared_ptr<Locations>>> m_locationPromise;
    std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> m_forecastPromise;
};

Q_DECLARE_TYPEINFO(WeatherData::DayForecastInfo, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(WeatherData::HourlyForecastInfo, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(WeatherData::WeatherEvent, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(WeatherData, Q_RELOCATABLE_TYPE);
