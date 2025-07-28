/*
    SPDX-FileCopyrightText: 2007-2009, 2019 Shawn Starr <shawn.starr@rogers.com>
    SPDX-FileCopyrightText: 2024 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/* Ion for NOAA's National Weather Service openAPI data */

#pragma once

#include "ion.h"

#include <KUnitConversion/Converter>
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
    QString locationName;
    QString stationID;
    double stationLatitude = qQNaN();
    double stationLongitude = qQNaN();
    QString stateName;
    QString countyID;
    QString forecastUrl;

    // Current observation information.
    struct Observation {
        QDateTime timestamp;
        QString weather;
        float temperature_F = qQNaN();
        float humidity = qQNaN();
        float windDirection = qQNaN();
        float windSpeed = qQNaN();
        float windGust = qQNaN();
        float pressure = qQNaN();
        float dewpoint_F = qQNaN();
        float heatindex_F = qQNaN();
        float windchill_F = qQNaN();
        float visibility = qQNaN();
        bool isNight = false;
    };
    Observation observation;

    struct Forecast {
        int day;
        QString summary;
        float low = qQNaN();
        float high = qQNaN();
        int precipitation = 0;
        bool isDayTime = true;
    };
    QList<Forecast> forecasts;

    struct Alert {
        QString headline;
        QString description;
        QString infoUrl;
        Warnings::PriorityClass priority;
        QDateTime startTime;
        QDateTime endTime;
    };
    QList<Alert> alerts;

    bool isObservationDataError = false;
    bool isPointsInfoDataError = false;
    bool isForecastsDataError = false;
    bool isAlertsDataError = false;
};

Q_DECLARE_TYPEINFO(WeatherData::Forecast, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(WeatherData, Q_RELOCATABLE_TYPE);

class NOAAIon : public Ion
{
    Q_OBJECT

public:
    NOAAIon(QObject *parent, const QVariantList &args);
    ~NOAAIon() override;

    void findPlaces(std::shared_ptr<QPromise<std::shared_ptr<Locations>>> promise, const QString &searchString) override;

    void fetchForecast(std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> promise, const QString &placeInfo) override;

private:
    void updateWeather();

    /* NOAA Methods - Internal for Ion */
    QMap<QString, ConditionIcons> setupConditionIconMappings() const;
    QMap<QString, ConditionIcons> const &conditionIcons() const;

    // Current Conditions Weather info
    // bool night(const QString& source);
    ConditionIcons getConditionIcon(const QString &weather, bool isDayTime) const;

    // Helper to make an API request
    using Callback = void (NOAAIon::*)(KJob *, const QByteArray &);
    KJob *requestAPIJob(const QUrl &url, Callback onResult);

    // Load and parse the station list
    void getStationList();
    void readStationList(QXmlStreamReader &xml);
    void parseStationID(QXmlStreamReader &xml);

    // Load and parse the observation data from a station
    void getObservation();
    void readObservation(KJob *job, const QByteArray &data);

    // Load and parse upcoming forecast for the next N days
    void getForecast();
    void readForecast(KJob *job, const QByteArray &data);

    // The NOAA API is based on grid of points
    void getPointsInfo();
    void readPointsInfo(KJob *job, const QByteArray &data);

    // Methods to get alerts.
    void getAlerts();
    void readAlerts(KJob *job, const QByteArray &data);

    // Utility method to parse XML data
    void parseUnknownElement(QXmlStreamReader &xml) const;

    // Utility methods to parse JSON data
    KUnitConversion::UnitId parseUnit(const QString &unitCode) const;
    float parseQV(const QJsonValue &qv, KUnitConversion::UnitId destUnit = KUnitConversion::InvalidUnit) const;
    QString windDirectionFromAngle(float degrees) const;
    QString i18nForecast(const QString &summary) const;

Q_SIGNALS:
    void locationUpdated();
    void observationUpdated();
    void pointsInfoUpdated();

private:
    struct StationInfo {
        QString stateName;
        QString stationName;
        QString stationID;
        QPointF location;
    };

    // Station list
    QHash<QString, StationInfo> m_places;

    // Weather information
    std::shared_ptr<WeatherData> m_weatherData;

    KUnitConversion::Converter m_converter;

    QSet<KJob *> m_jobs;

    // used to handle the old format of a forecast request string
    bool m_isLegacy = false;
    QString m_newPlaceInfo;

    std::shared_ptr<QPromise<std::shared_ptr<Locations>>> m_locationPromise;
    std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> m_forecastPromise;
};
