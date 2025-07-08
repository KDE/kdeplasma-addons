/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "forecast.h"

Forecast::Forecast()
    : m_error(false)
{
}

Forecast::~Forecast()
{
}

bool Forecast::isError() const
{
    return m_error;
}

QVariant Forecast::getStation() const
{
    if (m_station.has_value()) {
        return QVariant::fromValue(*m_station);
    }
    return {};
}

QVariant Forecast::getMetaData() const
{
    if (m_metaData.has_value()) {
        return QVariant::fromValue(*m_metaData);
    }
    return {};
}

QVariant Forecast::getCurrentDay() const
{
    if (m_currentDay.has_value()) {
        return QVariant::fromValue(*m_currentDay);
    }
    return {};
}

QVariant Forecast::getLastDay() const
{
    if (m_lastDay.has_value()) {
        return QVariant::fromValue(*m_lastDay);
    }
    return {};
}

QVariant Forecast::getLastObservation() const
{
    if (m_lastObservation.has_value()) {
        return QVariant::fromValue(*m_lastObservation);
    }
    return {};
}

FutureDays *Forecast::getFutureDays() const
{
    return m_futureDays.get();
}

Warnings *Forecast::getWarnings() const
{
    return m_warnings.get();
}

void Forecast::setError()
{
    m_error = true;
}

void Forecast::setStation(const Station &station)
{
    if (station.isDataPresent()) {
        m_station = station;
    }
}

void Forecast::setMetadata(const MetaData &metaData)
{
    if (metaData.isDataPresent()) {
        m_metaData = metaData;
    }
}

void Forecast::setCurrentDay(const CurrentDay &currentDay)
{
    if (currentDay.isDataPresent()) {
        m_currentDay = currentDay;
    }
}

void Forecast::setLastDay(const LastDay &lastDay)
{
    if (lastDay.isDataPresent()) {
        m_lastDay = lastDay;
    }
}

void Forecast::setLastObservation(const LastObservation &lastObservation)
{
    if (lastObservation.isDataPresent()) {
        m_lastObservation = lastObservation;
    }
}

void Forecast::setFutureDays(std::shared_ptr<FutureDays> futureDays)
{
    if (futureDays->columnCount()) {
        m_futureDays = futureDays;
    }
}
void Forecast::setWarnings(std::shared_ptr<Warnings> warnings)
{
    if (warnings->rowCount()) {
        m_warnings = warnings;
    }
}

#include "moc_forecast.cpp"
