/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QNetworkInformation>
#include <QObject>
#include <QQueue>
#include <QThread>

#include "forecastdata.h"
#include "locationsdata.h"

#include "ion.h"

class QNetworkAccessManager;

/*!
 * \class IonControl
 *
 * \brief The IonControl class updates forecasts and locations
 *
 *  Manages weather-related data updates (\c LocationsData and \c ForecastData) and handles
 *  asynchronous requests to fetch this data, coordinating with a provider's object.
 *  This class is created one per provider by \c WeatherDataMonitor .
 */
class IonControl : public QObject
{
    Q_OBJECT

    friend class WeatherDataMonitor;

public:
    /*!
     * Used to request a locations update
     */
    void updateLocations(const std::shared_ptr<LocationsData> &locationData);
    /*!
     * Used to request a forecast update
     */
    void updateForecast(const std::shared_ptr<ForecastData> &forecastData);

    ~IonControl() override;

private:
    /*!
     * \enum IonControl::NetworkStatus
     * This enum describes the current status of the network.
     *
     * \value Disconnected
     * The network is not available.
     *
     * \value Connecting
     * IonControl is trying to connect to the network.
     *
     * \value Connected
     * Successfully connected to the network.
     */
    enum ConnectionStatus {
        Disconnected = 0,
        Connecting,
        Connected,
        ProviderUnavailable,
    };

    Q_ENUM(ConnectionStatus)

    explicit IonControl(const QString &name, const std::shared_ptr<Ion> &ion, const std::shared_ptr<QThread> &ionThread, QObject *parent = nullptr);

    /*!
     * Check queues if there are any other requests to update locations or forecasts. If so
     * calls \c updateForecast or \c updateLocations respectively.
     */
    void checkQueues();

    /*!
     * Check if provider is available
     */
    void checkProviderAvailability();

Q_SIGNALS:
    /*!
     * Used to request a locations update from a provider(ion).
     */
    void fetchLocationsRequest(std::shared_ptr<QPromise<std::shared_ptr<Locations>>> promise, const QString &searchString);
    /*!
     * Used to request a forecast update from a provider(ion).
     */
    void fetchForecastRequest(std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> promise, const QString &place);

private Q_SLOTS:
    /*!
     * Whenever networking changes, take action
     */
    void onOnlineStateChanged(QNetworkInformation::Reachability reachability);

    /*!
     * Whenever provider availability changes, take action
     */
    void onProviderAvailabilityChanged();

    /*!
     * Called when provider(ion) ended locations update
     */
    void onFetchLocationsEnded();
    /*!
     * Called, if for some reason, locations update was cancelled
     */
    void onFetchLocationsCancelled();

    /*!
     * Called when provider(ion) ended forecast update
     */
    void onFetchForecastEnded();
    /*!
     * Called, if for some reason, forecast update was cancelled
     */
    void onFetchForecastCancelled();

private:
    QQueue<std::shared_ptr<LocationsData>> m_locationQueue;
    QQueue<std::shared_ptr<ForecastData>> m_forecastQueue;

    std::shared_ptr<LocationsData> m_currentLocationUpdate;
    std::shared_ptr<ForecastData> m_currentForecastUpdate;

    std::shared_ptr<QFutureWatcher<std::shared_ptr<Locations>>> m_locationsFutureWatcher;
    std::shared_ptr<QFutureWatcher<std::shared_ptr<Forecast>>> m_forecastFutureWatcher;

    std::shared_ptr<Ion> m_ion;
    std::shared_ptr<QThread> m_fetchThread;

    QTimer *m_networkReconnectTimer;
    QTimer *m_providerReconnectTimer;

    bool m_searchStringChanged = false;

    bool m_isBusy = false;

    QString m_ionName;

    int m_reconnectAttempts;

    ConnectionStatus m_connectionStatus;

    std::unique_ptr<QNetworkAccessManager> m_manager;
};
