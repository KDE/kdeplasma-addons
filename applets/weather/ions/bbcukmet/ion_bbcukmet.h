/*
    SPDX-FileCopyrightText: 2007-2009 Shawn Starr <shawn.starr@rogers.com>
    SPDX-FileCopyrightText: 2024 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/* Ion for BBC Weather from UKMET Office */

#pragma once

#include "ion.h"

#include <QDateTime>
#include <QList>

#include <forecast.h>
#include <locations.h>

class KJob;
namespace KIO
{
class Job;
}

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
    double stationLatitude = qQNaN();
    double stationLongitude = qQNaN();

    // Current observation information.
    struct Observation {
        QString obsTime;
        QDateTime observationDateTime;
        bool isNight = false;

        QString condition;
        QString conditionIcon;
        float temperature_C = qQNaN();
        QString windDirection;
        float windSpeed_miles = qQNaN();
        float humidity = qQNaN();
        float pressure = qQNaN();
        QString pressureTendency;
        QString visibilityStr;
    };
    Observation current;

    // Forecasts
    struct ForecastInfo {
        QDate period;
        bool isNight = false;
        QString iconName;
        QString summary;
        float tempHigh = qQNaN();
        float tempLow = qQNaN();
        float windSpeed = qQNaN();
        QString windDirection;
        int precipitationPct = 0;
    };

    QList<ForecastInfo> forecasts;
};

class UKMETIon : public Ion
{
    Q_OBJECT

public:
    explicit UKMETIon(QObject *parent, const QVariantList &args);
    ~UKMETIon() override;

    void findPlaces(std::shared_ptr<QPromise<std::shared_ptr<Locations>>> promise, const QString &searchString) override;

    void fetchForecast(std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> promise, const QString &placeInfo) override;

private:
    void updateWeather();

    /* UKMET Methods - Internal for Ion */
    QMap<QString, ConditionIcons> setupDayIconMappings() const;
    QMap<QString, ConditionIcons> setupNightIconMappings() const;
    QMap<QString, WindDirections> setupWindIconMappings() const;

    QMap<QString, ConditionIcons> const &nightIcons() const;
    QMap<QString, ConditionIcons> const &dayIcons() const;
    QMap<QString, WindDirections> const &windIcons() const;

    // General util methods to request API Calls
    // TODO: Very barebones. Abstract away to a class which can internally
    // handle the state of requests, pending calls, retries and server errors
    KJob *requestAPIJob(const QUrl &url, QByteArray &result);
    int secondsToRetry();

    // Load and Parse the place search listings
    void readSearchData(const QByteArray &json);
    void validate(); // Sync data source with Applet

    // Load and parse the weather forecast
    void getForecast();
    bool readForecast(const QJsonDocument &doc);
    WeatherData::ForecastInfo parseForecastReport(const QJsonObject &report, bool isNight) const;

    // Load and parse current observation data
    void getObservation();
    bool readObservationData(const QJsonDocument &doc);

    // Clear unneeded data
    void clearForecastData();
    void clearLocationData();

private Q_SLOTS:
    void places_slotJobFinished(KJob *job);
    void places_slotJobAutoFinished(KJob *job);

    void observation_slotJobFinished(KJob *job);
    void forecast_slotJobFinished(KJob *job);

private:
    QHash<QString, PlaceInfo> m_place;
    QList<QString> m_locations;

    // Locations information
    QString m_searchString;
    QByteArray m_locationsData;
    QByteArray m_locationsAutoData;

    // Weather information
    QString m_placeName;
    QString m_placeId;
    QByteArray m_forecastData;
    std::shared_ptr<WeatherData> m_weatherData;

    int m_pendingSearchCount = 0;
    int m_retryAttempts = 0;

    std::shared_ptr<QPromise<std::shared_ptr<Locations>>> m_locationPromise;
    std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> m_forecastPromise;
};

Q_DECLARE_TYPEINFO(WeatherData::ForecastInfo, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(WeatherData, Q_RELOCATABLE_TYPE);
