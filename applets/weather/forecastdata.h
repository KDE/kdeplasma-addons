/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QDateTime>
#include <QObject>
#include <QTimer>

#include "forecast.h"

/*!
 * \class ForecastData
 *
 * \brief Manages forecast for specific location
 *
 * Used to manage forecast, provide additional info about forecast and emit
 * \c forecastUpdated signal whenever forecast is updated. Created only by
 * \c WeatherDataMonitor.
 */
class ForecastData : public QObject
{
    Q_OBJECT

    friend class WeatherDataMonitor;
    friend class IonControl;

public:
    /*!
     * Return provider for which this ForecastData is related
     */
    QString provider() const;
    /*!
     * Return placeInfo by which forecast is updated
     */
    QString placeInfo() const;
    /*!
     * Return the last time there was an update of forecast
     */
    QDateTime lastUpdateTime() const;
    /*!
     * Return the forecast. If the forecast is empty return \c nullptr
     */
    const std::shared_ptr<Forecast> &forecast() const;

    ~ForecastData() override;

private:
    explicit ForecastData(const QString &provider, const QString &place);

    /*!
     * Set the forecast. Used only by \c IonControl.
     */
    void setForecast(const std::shared_ptr<Forecast> &forecast);

Q_SIGNALS:
    /*!
     * Emitted whenever the forecast are updated.
     */
    void forecastUpdated();
    /*!
     * Emitted when the ForecastData is about to be removed
     */
    void forecastDataRemoved(const QString &provider, const QString &place);

private:
    QDateTime m_lastUpdateTime;

    QString m_provider;

    QString m_placeInfo;

    std::shared_ptr<Forecast> m_forecast;
};
