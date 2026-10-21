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

class DWDIon : public Ion
{
    Q_OBJECT

public:
    explicit DWDIon(QObject *parent, const QVariantList &args);
    ~DWDIon() override;

    void findPlaces(std::shared_ptr<QPromise<std::shared_ptr<Locations>>> promise, const QString &searchString) override;

    void fetchForecast(std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> promise, const QString &placeInfo) override;

private:
    QMap<uint, ConditionIcons> getUniversalIcons() const;
    QMap<uint, ConditionIcons> setupDayIconMappings() const;
    QMap<uint, ConditionIcons> setupNightIconMappings() const;
    QMap<uint, WindDirections> setupWindIconMappings() const;

    QMap<uint, ConditionIcons> const &dayIcons() const;
    QMap<uint, ConditionIcons> const &nightIcons() const;
    QMap<uint, WindDirections> const &windIcons() const;

    KJob *requestAPIJob(const QUrl &url, QByteArray &result);

    void parseStationData(const QByteArray &data);
    void searchInStationList(const QString &place);

    void validate();

    bool parseForecastData(const QByteArray &data);
    void parseMeasureData(const QByteArray &data);
    void updateWeather();

    // Helper methods
    QString camelCaseString(const QString &text) const;
    int roundWindDirections(int windDirection) const;
    bool isNightTime(const QDateTime &dateTime) const;
    std::optional<qreal> parseNumber(const QVariant &number) const;
    std::optional<QDateTime> parseDateFromMSecs(const QVariant &timestamp) const;

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
    QDateTime m_sunriseTime;
    QDateTime m_sunsetTime;
    std::shared_ptr<Forecast> m_forecast;

    // used to handle the old format of a forecast request string
    bool m_isLegacy = false;

    bool m_isForecastsDataPending = false;
    bool m_isMeasureDataPending = false;
    bool m_forecastInvalid = false;

    std::shared_ptr<QPromise<std::shared_ptr<Locations>>> m_locationPromise;
    std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> m_forecastPromise;
};
