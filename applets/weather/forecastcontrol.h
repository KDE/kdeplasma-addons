/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QObject>

#include <QHash>
#include <QObjectBindableProperty>
#include <QTimer>

#include <qqmlintegration.h>

#include "forecastdata.h"

class WeatherDataMonitor;

/*!
 * \class ForecastControl
 *
 * \brief Request a forecast from a provider with specific placeInfo
 *
 * The ForecastControl class is used from qml to retrieve forecast and the status of update.
 * Also sets a timer to periodic update.
 */
class ForecastControl : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(Status status READ default WRITE default NOTIFY statusChanged BINDABLE bindableStatus)

    Q_PROPERTY(QVariant forecast READ getForecast NOTIFY forecastChanged)

public:
    enum Status {
        Normal = 0,
        Connecting,
        NeedsConfiguration,
        Timeout,
    };
    Q_ENUM(Status);

    explicit ForecastControl(QObject *parent = nullptr);

    ~ForecastControl() override;

    /*!
     * Sets the interval at which the forecast will be updated
     *
     * \a minutes interval in minutes
     */
    Q_INVOKABLE void setUpdateInterval(int minutes);
    /*!
     * Set the provider and placeInfo by which forecast will be retrieved
     *
     * \a provider A provider name
     * \a placeInfo A specific string by which the provider understand the location for which the forecast can be received
     */
    Q_INVOKABLE bool setForecastLocation(const QString &provider, const QString &placeInfo);

private:
    /*!
     * Return \c true if forecast is outdated and false otherwise
     */
    bool isDataOutdated() const;

    QBindable<Status> bindableStatus();

    QVariant getForecast();

    /*!
     * Called whenever forecast need to be updated
     */
    void updateForecastInfo();

Q_SIGNALS:
    /*!
     * Emitted when forecast was updated
     */
    void forecastChanged();
    void statusChanged();

private Q_SLOTS:
    void onWeatherDataUpdated();

private:
    QTimer *m_updateTimer;

    std::shared_ptr<ForecastData> m_forecastData;

    std::shared_ptr<WeatherDataMonitor> m_weatherDataMonitor;

    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(ForecastControl, Status, m_status, NeedsConfiguration, &ForecastControl::statusChanged);
};
