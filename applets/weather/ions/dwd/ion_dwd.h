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

#pragma once

#include "ion.h"

#include <QList>

#include <forecast.h>
#include <locations.h>

class KJob;
namespace KIO
{
class Job;
}

class WeatherData
{
public:
    QString place;

    // Current observation information.
    QDateTime observationDateTime;
    QDateTime sunriseTime;
    QDateTime sunsetTime;

    QString condIconNumber;
    QString windDirection;
    float temperature = qQNaN();
    float humidity = qQNaN();
    float pressure = qQNaN();
    float windSpeed = qQNaN();
    float gustSpeed = qQNaN();
    float dewpoint = qQNaN();

    // If current observations not available, use forecast data for current day
    QString windDirectionAlt;
    float windSpeedAlt = qQNaN();
    float gustSpeedAlt = qQNaN();

    // 7 forecast
    struct ForecastInfo {
        QDateTime period;
        QString iconName;
        QString summary;
        float tempHigh = qQNaN();
        float tempLow = qQNaN();
        int precipitation = 0;
        float windSpeed = qQNaN();
        QString windDirection;
    };

    // 7 day forecast
    QList<ForecastInfo> forecasts;

    struct WarningInfo {
        QString type;
        Warnings::PriorityClass priority = Warnings::Low;
        QString headline;
        QString description;
        QDateTime timestamp;
    };

    QList<WarningInfo> warnings;

    bool isForecastsDataPending = false;
    bool isMeasureDataPending = false;
};

class DWDIon : public Ion
{
    Q_OBJECT

public:
    explicit DWDIon(QObject *parent, const QVariantList &args);
    ~DWDIon() override;

    void findPlaces(std::shared_ptr<QPromise<std::shared_ptr<Locations>>> promise, const QString &searchString) override;

    void fetchForecast(std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> promise, const QString &placeInfo) override;

private:
    QMap<QString, ConditionIcons> getUniversalIcons() const;
    QMap<QString, ConditionIcons> setupDayIconMappings() const;
    QMap<QString, ConditionIcons> setupNightIconMappings() const;
    QMap<QString, WindDirections> setupWindIconMappings() const;

    QMap<QString, ConditionIcons> const &dayIcons() const;
    QMap<QString, ConditionIcons> const &nightIcons() const;
    QMap<QString, WindDirections> const &windIcons() const;

    KJob *requestAPIJob(const QUrl &url, QByteArray &result);

    void parseStationData(const QByteArray &data);
    void searchInStationList(const QString &place);

    void validate();

    void parseForecastData(const QJsonDocument &doc);
    void parseMeasureData(const QJsonDocument &doc);
    void updateWeather();

    // Helper methods
    QString camelCaseString(const QString &text) const;
    QString roundWindDirections(int windDirection) const;
    bool isNightTime(const WeatherData &weatherData) const;
    float parseNumber(const QVariant &number) const;
    QDateTime parseDateFromMSecs(const QVariant &timestamp) const;

    // Clear unneeded data
    void clearForecastData();
    void clearLocationData();

private Q_SLOTS:
    void places_slotJobFinished(KJob *job);
    void measure_slotJobFinished(KJob *job);
    void forecast_slotJobFinished(KJob *job);

private:
    // Key dicts
    QMap<QString, QString> m_place;
    QList<QString> m_locations;

    // Locations information
    QByteArray m_locationsData;
    QString m_searchString;

    // Weather information
    QString m_weatherName;
    QString m_weatherID;
    QByteArray m_forecastData;
    QByteArray m_measurementData;
    std::shared_ptr<WeatherData> m_weatherData;

    // used to handle the old format of a forecast request string
    bool m_isLegacy = false;

    std::shared_ptr<QPromise<std::shared_ptr<Locations>>> m_locationPromise;
    std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> m_forecastPromise;
};
