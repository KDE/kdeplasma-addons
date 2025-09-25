/*
    SPDX-FileCopyrightText: 2007-2009 Shawn Starr <shawn.starr@rogers.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ion.h"

#include <KHolidays/SunEvents>

Ion::Ion(QObject *parent)
    : QObject(parent)
{
}

Ion::~Ion()
{
}

/**
 * Return wind direction svg element to display in applet when given a wind direction.
 */
QString Ion::getWindDirectionIcon(const QMap<QString, WindDirections> &windDirList, const QString &windDirection) const
{
    switch (windDirList[windDirection.toLower()]) {
    case N:
        return QStringLiteral("N");
    case NNE:
        return QStringLiteral("NNE");
    case NE:
        return QStringLiteral("NE");
    case ENE:
        return QStringLiteral("ENE");
    case E:
        return QStringLiteral("E");
    case SSE:
        return QStringLiteral("SSE");
    case SE:
        return QStringLiteral("SE");
    case ESE:
        return QStringLiteral("ESE");
    case S:
        return QStringLiteral("S");
    case NNW:
        return QStringLiteral("NNW");
    case NW:
        return QStringLiteral("NW");
    case WNW:
        return QStringLiteral("WNW");
    case W:
        return QStringLiteral("W");
    case SSW:
        return QStringLiteral("SSW");
    case SW:
        return QStringLiteral("SW");
    case WSW:
        return QStringLiteral("WSW");
    case VR:
        return QStringLiteral("VR"); // For now, we'll make a variable wind icon later on
    }

    // No icon available, use 'X'
    return QString();
}

/**
 * Return weather icon to display in an applet when given a condition.
 */
QString Ion::getWeatherIcon(ConditionIcons condition) const
{
    switch (condition) {
    case ClearDay:
        return QStringLiteral("weather-clear-symbolic");
    case ClearWindyDay:
        return QStringLiteral("weather-clear-wind-symbolic");
    case FewCloudsDay:
        return QStringLiteral("weather-few-clouds-symbolic");
    case FewCloudsWindyDay:
        return QStringLiteral("weather-few-clouds-wind-symbolic");
    case PartlyCloudyDay:
        return QStringLiteral("weather-clouds-symbolic");
    case PartlyCloudyWindyDay:
        return QStringLiteral("weather-clouds-wind-symbolic");
    case Overcast:
        return QStringLiteral("weather-overcast-symbolic");
    case OvercastWindy:
        return QStringLiteral("weather-overcast-wind-symbolic");
    case Rain:
        return QStringLiteral("weather-showers-symbolic");
    case LightRain:
        return QStringLiteral("weather-showers-scattered-symbolic");
    case Showers:
        return QStringLiteral("weather-showers-scattered-symbolic");
    case ChanceShowersDay:
        return QStringLiteral("weather-showers-scattered-day-symbolic");
    case ChanceShowersNight:
        return QStringLiteral("weather-showers-scattered-night-symbolic");
    case ChanceSnowDay:
        return QStringLiteral("weather-snow-scattered-day-symbolic");
    case ChanceSnowNight:
        return QStringLiteral("weather-snow-scattered-night-symbolic");
    case Thunderstorm:
        return QStringLiteral("weather-storm-symbolic");
    case Hail:
        return QStringLiteral("weather-hail-symbolic");
    case Snow:
        return QStringLiteral("weather-snow-symbolic");
    case LightSnow:
        return QStringLiteral("weather-snow-scattered-symbolic");
    case Flurries:
        return QStringLiteral("weather-snow-scattered-symbolic");
    case RainSnow:
        return QStringLiteral("weather-snow-rain-symbolic");
    case FewCloudsNight:
        return QStringLiteral("weather-few-clouds-night-symbolic");
    case FewCloudsWindyNight:
        return QStringLiteral("weather-few-clouds-wind-night-symbolic");
    case PartlyCloudyNight:
        return QStringLiteral("weather-clouds-night-symbolic");
    case PartlyCloudyWindyNight:
        return QStringLiteral("weather-clouds-wind-night-symbolic");
    case ClearNight:
        return QStringLiteral("weather-clear-night-symbolic");
    case ClearWindyNight:
        return QStringLiteral("weather-clear-wind-night-symbolic");
    case Mist:
        return QStringLiteral("weather-fog-symbolic");
    case Haze:
        return QStringLiteral("weather-fog-symbolic");
    case FreezingRain:
        return QStringLiteral("weather-freezing-rain-symbolic");
    case FreezingDrizzle:
        return QStringLiteral("weather-freezing-rain-symbolic");
    case ChanceThunderstormDay:
        return QStringLiteral("weather-storm-day-symbolic");
    case ChanceThunderstormNight:
        return QStringLiteral("weather-storm-night-symbolic");
    case NotAvailable:
        return QStringLiteral("weather-none-available-symbolic");
    }
    return QStringLiteral("weather-none-available-symbolic");
}

/**
 * Return weather icon to display in an applet when given a condition.
 */
QString Ion::getWeatherIcon(const QMap<QString, ConditionIcons> &conditionList, const QString &condition) const
{
    return getWeatherIcon(conditionList[condition.toLower()]);
}

bool Ion::isNightTime(const QDateTime &dateTime, double latitude, double longitude)
{
    if (!dateTime.isValid() || qIsNaN(latitude) || qIsNaN(longitude)) {
        return false;
    }

    const auto events = KHolidays::SunEvents(dateTime, latitude, longitude);

    if (events.isPolarDay()) {
        return false;
    }
    if (events.isPolarNight()) {
        return true;
    }
    if (events.isPolarTwilight()) {
        return dateTime < events.civilDawn() || dateTime > events.civilDusk();
    }

    return dateTime < events.sunrise() || dateTime > events.sunset();
}

#include "moc_ion.cpp"
