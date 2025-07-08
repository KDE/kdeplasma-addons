/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "forecastcontrol.h"

#include "weathercontroller_debug.h"

#include "weatherdatamonitor_p.h"

// Constructor
ForecastControl::ForecastControl(QObject *parent)
    : QObject(parent)
{
    qCDebug(WEATHER::CONTROLLER) << "ForecastControl: start initializing";
    m_updateTimer = new QTimer(this);
    m_updateTimer->setSingleShot(false);
    connect(m_updateTimer, &QTimer::timeout, this, [this]() {
        qCDebug(WEATHER::CONTROLLER) << "ForecastControl: UpdateTimer: start updating forecast info";
        updateForecastInfo();
    });

    qCDebug(WEATHER::CONTROLLER) << "ForecastControl: get instance of WeatherDataMonitor";
    m_weatherDataMonitor = WeatherDataMonitor::instance();
    qCDebug(WEATHER::CONTROLLER) << "ForecastControl: successfully initialized";
}

QVariant ForecastControl::getForecast()
{
    if (m_forecastData) {
        return QVariant::fromValue(*m_forecastData->forecast());
    }
    return {};
}

// Destructor
ForecastControl::~ForecastControl()
{
    qCDebug(WEATHER::CONTROLLER) << "ForecastControl: destroying";
}

QBindable<ForecastControl::Status> ForecastControl::bindableStatus()
{
    return &m_status;
}

void ForecastControl::onWeatherDataUpdated()
{
    qCDebug(WEATHER::CONTROLLER) << "ForecastControl: weatherDataUpdated signal received";
    if (m_forecastData->forecast()->isError()) {
        qCDebug(WEATHER::CONTROLLER) << "ForecastControl: forecast data didn't updated due to error";
        m_status = Timeout;
        Q_EMIT forecastChanged();
    }

    m_status = Normal;

    Q_EMIT forecastChanged();

    qCDebug(WEATHER::CONTROLLER) << "ForecastControl: weatherDataUpdated successfully updated";
}

void ForecastControl::setUpdateInterval(int minutes)
{
    // stop forecast update if minutes is zero
    if (minutes == 0) {
        if (m_updateTimer->isActive()) {
            qCDebug(WEATHER::CONTROLLER) << "ForecastControl: stop active timer";
            m_updateTimer->stop();
        }
        return;
    }

    qCDebug(WEATHER::CONTROLLER) << "ForecastControl: set new update interval";
    m_updateTimer->setInterval(std::chrono::minutes(minutes));
    m_updateTimer->start();
}

bool ForecastControl::setForecastLocation(const QString &provider, const QString &place)
{
    qCDebug(WEATHER::CONTROLLER) << "ForecastControl: set new forecast location";
    if (m_forecastData) {
        qCDebug(WEATHER::CONTROLLER) << "ForecastControl: remove old forecast location";
        m_forecastData.reset();
    }

    m_forecastData = m_weatherDataMonitor->getForecastData(provider, place);

    if (!m_forecastData) {
        qCDebug(WEATHER::CONTROLLER) << "ForecastControl: error when receiving new forecastData from weatherDataMonitor";
        return false;
    }

    connect(m_forecastData.get(), &ForecastData::forecastUpdated, this, &ForecastControl::onWeatherDataUpdated);

    updateForecastInfo();

    qCDebug(WEATHER::CONTROLLER) << "ForecastControl: forecast data successfully updated";

    return true;
}

void ForecastControl::updateForecastInfo()
{
    qCDebug(WEATHER::CONTROLLER) << "ForecastControl: begin update forecast info";

    m_status = Connecting;

    if (!m_forecastData) {
        qCDebug(WEATHER::CONTROLLER) << "ForecastControl: error: forecastData is empty";
        m_status = Timeout;
        return;
    }

    // Check is forecast is outdated before request update. It is possible that the forecast
    // was update from another applet instance.
    if (!isDataOutdated()) {
        qCDebug(WEATHER::CONTROLLER) << "ForecastControl: forecast is not outdated. Skipping";
        m_status = Normal;
        Q_EMIT forecastChanged();
        return;
    }

    m_weatherDataMonitor->updateForecastData(m_forecastData);

    qCDebug(WEATHER::CONTROLLER) << "ForecastControl: successfully updated";

    Q_EMIT forecastChanged();
}

bool ForecastControl::isDataOutdated() const
{
    qCDebug(WEATHER::CONTROLLER) << "ForecastControl: check if forecast data is outdated";

    QDateTime lastUpdate = m_forecastData->lastUpdateTime();

    if (!lastUpdate.isValid()) {
        qCDebug(WEATHER::CONTROLLER) << "ForecastControl: forecastData contains invalid update time! Updating";
        return true;
    }

    lastUpdate = lastUpdate.addMSecs(m_updateTimer->interval());

    return lastUpdate <= QDateTime::currentDateTime();
}

#include "moc_forecastcontrol.cpp"
