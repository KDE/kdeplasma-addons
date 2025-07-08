/*
    SPDX-FileCopyrightText: 2007-2009 Shawn Starr <shawn.starr@rogers.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QPromise>
#include <QString>
#include <QThread>
#include <QtPlugin>

#include "forecast.h"
#include "locations.h"

/*!
 * \class Ion
 *
 * \brief Interface to implement weather provider
 *
 * The main class to create a weather ion. The ion is a class that is used to request
 * locations and forecast on the locations. When inherit the main methods must be implemented:
 * \c findPlaces and \c fetchForecast to receive locations and forecasts respectively. The created
 * class must be installed as Qt Plugin to be properly loaded and used. Also, the plugin must have
 * metadata added through \c Q_PLUGIN_METADATA with next values:
 * * name: the name of the class, string.
 * * quality: the quality of the class, int.
 * Without metadata a class is considered invalid and is skipped by \c WeatherDataMonitor
 */
class Ion : public QObject
{
    Q_OBJECT

public:
    enum ConditionIcons {
        ClearDay = 1,
        ClearWindyDay,
        FewCloudsDay,
        FewCloudsWindyDay,
        PartlyCloudyDay,
        PartlyCloudyWindyDay,
        Overcast,
        OvercastWindy,
        Rain,
        LightRain,
        Showers,
        ChanceShowersDay,
        Thunderstorm,
        Hail,
        Snow,
        LightSnow,
        Flurries,
        FewCloudsNight,
        FewCloudsWindyNight,
        ChanceShowersNight,
        PartlyCloudyNight,
        PartlyCloudyWindyNight,
        ClearNight,
        ClearWindyNight,
        Mist,
        Haze,
        FreezingRain,
        RainSnow,
        FreezingDrizzle,
        ChanceThunderstormDay,
        ChanceThunderstormNight,
        ChanceSnowDay,
        ChanceSnowNight,
        NotAvailable,
    };

    enum WindDirections {
        N,
        NNE,
        NE,
        ENE,
        E,
        SSE,
        SE,
        ESE,
        S,
        NNW,
        NW,
        WNW,
        W,
        SSW,
        SW,
        WSW,
        VR,
    };

    /*!
     * Constructor for the ion
     * \a parent The parent object.
     */
    explicit Ion(QObject *parent = nullptr);
    /*!
     * Destructor for the ion
     */
    ~Ion() override;

public Q_SLOTS:
    /*!
     * Slot which is used to request locations.
     *
     * \a promise the promise which is used to return the locations
     * \a searchString search string to search for location
     */
    virtual void findPlaces(std::shared_ptr<QPromise<std::shared_ptr<Locations>>> promise, const QString &searchString) = 0;

    /*!
     * Slot which is used to request forecast
     *
     * \a promise the promise which is used to return the forecast
     * \a placeInfo a specific string which is used to request a forecast
     */
    virtual void fetchForecast(std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> promise, const QString &placeInfo) = 0;

protected:
    /*!
     * Returns weather icon filename to display in applet.
     *
     * \a condition the current condition being reported.
     */
    QString getWeatherIcon(ConditionIcons condition) const;

    /*!
     * Returns weather icon filename to display in applet.
     *
     * \a conditionList a QList map pair of icons mapped to a enumeration of conditions.
     * \a condition the current condition being reported.
     */
    QString getWeatherIcon(const QMap<QString, ConditionIcons> &conditionList, const QString &condition) const;

    /*!
     * Returns wind icon element to display in applet.
     *
     * \a windDirList a QList map pair of wind directions mapped to a enumeration of directions.
     * \a windDirection the current wind direction.
     */
    QString getWindDirectionIcon(const QMap<QString, WindDirections> &windDirList, const QString &windDirection) const;

    static bool isNightTime(const QDateTime &dateTime, double latitude, double longitude);
};

Q_DECLARE_INTERFACE(Ion, "org.kde.weather.Ion")
