/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "lastobservation.h"

#include "klocalizedstring.h"

LastObservation::LastObservation()
    : m_isDataPresent(false)
{
}

LastObservation::~LastObservation()
{
}

bool LastObservation::isDataPresent() const
{
    return m_isDataPresent;
}

QVariant LastObservation::observationTimestamp() const
{
    if (m_observationTimestamp.has_value()) {
        return *m_observationTimestamp;
    }
    return {};
}

QVariant LastObservation::currentConditions() const
{
    if (m_currentConditions.has_value()) {
        return *m_currentConditions;
    }
    return {};
}

QVariant LastObservation::conditionIcon() const
{
    if (m_conditionIcon.has_value()) {
        return *m_conditionIcon;
    }
    return {};
}

QVariant LastObservation::temperature() const
{
    if (m_temperature.has_value()) {
        return *m_temperature;
    }
    return {};
}

QVariant LastObservation::windchill() const
{
    if (m_windchill.has_value()) {
        return *m_windchill;
    }
    return {};
}

QVariant LastObservation::heatIndex() const
{
    if (m_heatIndex.has_value()) {
        return *m_heatIndex;
    }
    return {};
}

QVariant LastObservation::humidex() const
{
    if (m_humidex.has_value()) {
        return *m_humidex;
    }
    if (m_humidexString.has_value()) {
        return *m_humidexString;
    }
    return {};
}

QVariant LastObservation::windSpeed() const
{
    if (m_windSpeed.has_value()) {
        return *m_windSpeed;
    }
    return {};
}

QVariant LastObservation::windGust() const
{
    if (m_windGust.has_value()) {
        return *m_windGust;
    }
    return {};
}

QVariant LastObservation::windDirection() const
{
    if (m_windDirection.has_value()) {
        return *m_windDirection;
    }
    return {};
}

QVariant LastObservation::visibility() const
{
    if (m_visibility.has_value()) {
        return *m_visibility;
    }
    if (m_visibilityString.has_value()) {
        return *m_visibilityString;
    }
    return {};
}

QVariant LastObservation::pressure() const
{
    if (m_pressure.has_value()) {
        return *m_pressure;
    }
    return {};
}

QVariant LastObservation::pressureTendency() const
{
    if (m_pressureTendency.has_value()) {
        return QVariant::fromValue(*m_pressureTendency);
    }
    return {};
}

QVariant LastObservation::UVIndex() const
{
    if (m_UVIndex.has_value()) {
        return *m_UVIndex;
    }
    return {};
}

QVariant LastObservation::UVRating() const
{
    if (m_UVRating.has_value()) {
        return *m_UVRating;
    }
    return {};
}

QVariant LastObservation::humidity() const
{
    if (m_humidity.has_value()) {
        return *m_humidity;
    }
    return {};
}

QVariant LastObservation::dewpoint() const
{
    if (m_dewpoint.has_value()) {
        return *m_dewpoint;
    }
    return {};
}

void LastObservation::setObservationTimestamp(const QDateTime &observationTimestamp)
{
    m_observationTimestamp = observationTimestamp;
    m_isDataPresent = true;
}

void LastObservation::setCurrentConditions(const QString &currentConditions)
{
    m_currentConditions = currentConditions;
    m_isDataPresent = true;
}

void LastObservation::setConditionIcon(const QString &conditionIcon)
{
    m_conditionIcon = conditionIcon;
    m_isDataPresent = true;
}

void LastObservation::setTemperature(qreal temperature)
{
    m_temperature = temperature;
    m_isDataPresent = true;
}

void LastObservation::setWindchill(qreal windchill)
{
    m_windchill = windchill;
    m_isDataPresent = true;
}

void LastObservation::setHeatIndex(qreal heatIndex)
{
    m_heatIndex = heatIndex;
    m_isDataPresent = true;
}

void LastObservation::setHumidex(int humidex)
{
    m_humidex = humidex;
    m_isDataPresent = true;
}

void LastObservation::setHumidex(const QString &humidexString)
{
    m_humidexString = humidexString;
    m_isDataPresent = true;
}

void LastObservation::setWindSpeed(qreal windSpeed)
{
    m_windSpeed = windSpeed;
    m_isDataPresent = true;
}

void LastObservation::setWindGust(qreal windGust)
{
    m_windGust = windGust;
    m_isDataPresent = true;
}

void LastObservation::setWindDirection(const QString &windDirection)
{
    m_windDirection = windDirection;
    m_isDataPresent = true;
}

void LastObservation::setVisibility(const QString &visibilityString)
{
    m_visibilityString = visibilityString;
    m_isDataPresent = true;
}

void LastObservation::setVisibility(qreal visibility)
{
    m_visibility = visibility;
    m_isDataPresent = true;
}

void LastObservation::setPressure(qreal pressure)
{
    m_pressure = pressure;
    m_isDataPresent = true;
}

void LastObservation::setPressureTendency(const QString &pressureTendency)
{
    m_pressureTendency = pressureTendency;
    m_isDataPresent = true;
}

void LastObservation::setUVIndex(int UVIndex)
{
    // Calculation of UVRating based on https://en.wikipedia.org/wiki/Ultraviolet_index#/media/File:UV_Index_NYC.png
    // UVIndex can be only from 0 to 10 so don`t assign if the value is out of the range
    if (UVIndex == 0) {
        m_UVRating = i18n("Low");
    } else if (UVIndex > 0 && UVIndex < 3) {
        m_UVRating = i18n("Moderate");
    } else if (UVIndex >= 3 && UVIndex < 6) {
        m_UVRating = i18n("High");
    } else if (UVIndex >= 6 && UVIndex < 8) {
        m_UVRating = i18n("Very high");
    } else if (UVIndex >= 8 && UVIndex < 11) {
        m_UVRating = i18n("Extreme");
    } else {
        return;
    }
    m_UVIndex = UVIndex;
    m_isDataPresent = true;
}

void LastObservation::setHumidity(qreal humidity)
{
    m_humidity = humidity;
    m_isDataPresent = true;
}

void LastObservation::setDewpoint(qreal dewpoint)
{
    m_dewpoint = dewpoint;
    m_isDataPresent = true;
}

#include "moc_lastobservation.cpp"
