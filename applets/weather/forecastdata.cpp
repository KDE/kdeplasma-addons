/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "forecastdata.h"

ForecastData::ForecastData(const QString &provider, const QString &place)
    : m_provider(provider)
    , m_placeInfo(place)
{
}

ForecastData::~ForecastData()
{
    Q_EMIT forecastDataRemoved(m_provider, m_placeInfo);
}

QString ForecastData::provider() const
{
    return m_provider;
}

QString ForecastData::placeInfo() const
{
    return m_placeInfo;
}

QDateTime ForecastData::lastUpdateTime() const
{
    return m_lastUpdateTime;
}

const std::shared_ptr<Forecast> &ForecastData::forecast() const
{
    return m_forecast;
}

void ForecastData::setForecast(const std::shared_ptr<Forecast> &forecast)
{
    m_forecast = forecast;
    m_lastUpdateTime = QDateTime::currentDateTime();
    Q_EMIT forecastUpdated();
}
