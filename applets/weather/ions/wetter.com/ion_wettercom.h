/*
    SPDX-FileCopyrightText: 2009 Thilo-Alexander Ginkel <thilo@ginkel.com>

    Based upon BBC Weather Ion by Shawn Starr
    SPDX-FileCopyrightText: 2007-2009 Shawn Starr <shawn.starr@rogers.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/* Ion for weather data from wetter.com */

#pragma once

#include "ion.h"

#include <QDateTime>
#include <QList>
#include <QObject>

#include <KIO/TransferJob>

#include <forecast.h>
#include <locations.h>

// wetter.com API project data
#define PROJECTNAME "weatherion"
#define PROVIDER_URL "https://www.wetter.com/"
#define SEARCH_URL "https://api.wetter.com/location/index/search/%1/project/" PROJECTNAME "/cs/%2"
#define FORECAST_URL "https://api.wetter.com/forecast/weather/city/%1/project/" PROJECTNAME "/cs/%2"
#define APIKEY "07025b9a22b4febcf8e8ec3e6f1140e8"
#define MIN_POLL_INTERVAL 3600000L // 1 h

class KJob;
namespace KIO
{
class Job;
}
class QXmlStreamReader;

class PlaceInfo
{
public:
    QString station;
    QString displayName;
    QString placeCode;
};

class WeatherData
{
public:
    QString place;
    QString stationName;

    // time difference to UTC
    int timeDifference;

    // credits as returned from API request
    QString credits;
    QString creditsUrl;

    class ForecastInfo
    {
    public:
        int tempHigh;
        int tempLow;

        QDateTime period;
        QString iconName;
        QString summary;
        int probability;
    };

    class ForecastPeriod : public ForecastInfo
    {
    public:
        ~ForecastPeriod();

        ForecastInfo getDayWeather() const;
        ForecastInfo getNightWeather() const;

        bool hasNightWeather() const;

        QList<std::shared_ptr<ForecastInfo>> dayForecasts;
        QList<std::shared_ptr<ForecastInfo>> nightForecasts;

    private:
        int getMaxTemp(const QList<std::shared_ptr<ForecastInfo>> &forecastInfos) const;
        int getMinTemp(const QList<std::shared_ptr<ForecastInfo>> &forecastInfos) const;
    };

    QList<std::shared_ptr<ForecastPeriod>> forecasts;
};

class WetterComIon : public Ion
{
    Q_OBJECT

public:
    explicit WetterComIon(QObject *parent, const QVariantList &args);
    ~WetterComIon() override;

    QUrl providerURL() const override;

    void findPlaces(std::shared_ptr<QPromise<std::shared_ptr<Locations>>> promise, const QString &searchString) override;

    void fetchForecast(std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> promise, const QString &placeInfo) override;

private:
    // Set up the mapping from the wetter.com condition code to the respective icon / condition name
    QMap<QString, ConditionIcons> setupCommonIconMappings() const;
    QMap<QString, ConditionIcons> setupDayIconMappings() const;
    QMap<QString, ConditionIcons> setupNightIconMappings() const;
    QHash<QString, QString> setupCommonConditionMappings() const;
    QHash<QString, QString> setupDayConditionMappings() const;
    QHash<QString, QString> setupNightConditionMappings() const;

    // Retrieve the mapping from the wetter.com condition code to the respective icon / condition name
    QMap<QString, ConditionIcons> const &nightIcons() const;
    QMap<QString, ConditionIcons> const &dayIcons() const;
    QHash<QString, QString> const &dayConditions() const;
    QHash<QString, QString> const &nightConditions() const;

    QString getWeatherCondition(const QHash<QString, QString> &conditionList, const QString &condition) const;

    // Find place
    void parseSearchResults(QXmlStreamReader &xml);
    void updatePlaces(const QList<PlaceInfo> &locations);

    // Retrieve and parse forecast
    void parseWeatherForecast(QXmlStreamReader &xml);
    void updateWeather();

private Q_SLOTS:
    void places_slotDataArrived(KIO::Job *job, const QByteArray &data);
    void places_slotJobFinished(KJob *job);

    void forecast_slotDataArrived(KIO::Job *job, const QByteArray &data);
    void forecast_slotJobFinished(KJob *job);

private:
    // Weather information
    PlaceInfo m_place;
    WeatherData m_weatherData;

    // Store KIO job
    std::unique_ptr<QXmlStreamReader> m_jobXml;

    // used to handle the old format of a forecast request string
    bool m_isLegacy = false;
    QString m_newPlaceInfo;

    std::shared_ptr<QPromise<std::shared_ptr<Locations>>> m_locationPromise;
    std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> m_forecastPromise;
};

Q_DECLARE_TYPEINFO(WeatherData::ForecastInfo, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(WeatherData::ForecastPeriod, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(WeatherData, Q_RELOCATABLE_TYPE);
