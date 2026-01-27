/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ioncontrol_p.h"

#include "weathercontroller_debug.h"
#include "weatherdatamonitor_p.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPromise>

using namespace std::chrono_literals;

const auto NETWORK_RECONNECT_DELAY = 1s;
const auto PROVIDER_RECONNECT_DELAY = 1min;
const auto MAX_RECONNECT_ATTEMPTS = 3;

IonControl::IonControl(const QString &name, const std::shared_ptr<Ion> &ion, const std::shared_ptr<QThread> &ionThread, QObject *parent)
    : QObject(parent)
    , m_ion(ion)
    , m_fetchThread(ionThread)
    , m_ionName(name)
    , m_reconnectAttempts(0)
    , m_connectionStatus(Connecting)
{
    m_manager = std::make_unique<QNetworkAccessManager>();

    qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": The worker thread is: " << m_fetchThread.get();

    // additional delay to allow the network to establish properly
    m_networkReconnectTimer = new QTimer(this);
    m_networkReconnectTimer->setSingleShot(true);
    m_networkReconnectTimer->setInterval(NETWORK_RECONNECT_DELAY);
    connect(m_networkReconnectTimer, &QTimer::timeout, this, [this]() {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": ReconnectTimer: start updating forecast info";
        m_connectionStatus = Connecting;
        // First check if provider available.
        checkProviderAvailability();
    });

    m_providerReconnectTimer = new QTimer(this);
    m_providerReconnectTimer->setSingleShot(true);
    m_providerReconnectTimer->setInterval(PROVIDER_RECONNECT_DELAY);
    connect(m_providerReconnectTimer, &QTimer::timeout, this, [this]() {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": ReconnectTimer: start reconnect attempt";
        checkProviderAvailability();
    });

    connect(this, &IonControl::fetchLocationsRequest, ion.get(), &Ion::findPlaces);
    connect(this, &IonControl::fetchForecastRequest, ion.get(), &Ion::fetchForecast);

    QNetworkInformation::loadBackendByFeatures(QNetworkInformation::Feature::Reachability);

    if (const auto instance = QNetworkInformation::instance()) {
        connect(instance, &QNetworkInformation::reachabilityChanged, this, &IonControl::onOnlineStateChanged);
        if (instance->reachability() == QNetworkInformation::Reachability::Online) {
            m_connectionStatus = Connected;
        } else {
            m_connectionStatus = Disconnected;
        }
        onOnlineStateChanged(instance->reachability());
    }

    qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": Initialized";
}

IonControl::~IonControl()
{
    qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": destroying";

    m_currentLocationUpdate.reset();
    m_currentForecastUpdate.reset();

    if (m_locationsFutureWatcher) {
        m_locationsFutureWatcher->cancel();
        m_locationsFutureWatcher.reset();
    }

    if (m_forecastFutureWatcher) {
        m_forecastFutureWatcher->cancel();
        m_forecastFutureWatcher.reset();
    }

    if (m_fetchThread) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": remove fetch thread";
        m_fetchThread->quit();
        m_fetchThread->wait();
        m_fetchThread.reset();
    }

    qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << " process events";

    // Check for pending events before destroying, as m_fetchThread may still
    // have queued events that could cause a crash.
    thread()->eventDispatcher()->processEvents(QEventLoop::AllEvents);

    qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": destroyed";
}

void IonControl::updateLocations(const std::shared_ptr<LocationsData> &locationData)
{
    qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": get locations request";

    if (m_locationQueue.contains(locationData)) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": locationData already in queue. Don't add another one";
        return;
    }

    if (m_connectionStatus == ProviderUnavailable) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": Provider is unreachable. Skip update locations";
        m_locationQueue.enqueue(locationData);
        checkProviderAvailability();
        return;
    }

    // skip update and add locations to the queue if the network is offline
    if (m_connectionStatus == Disconnected) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": Network connection is unavailable. Skip update locations";
        auto errorLocations = std::make_shared<Locations>();
        errorLocations->setError();
        locationData->setLocations(errorLocations);
        m_locationQueue.enqueue(locationData);
        return;
    }

    if (m_connectionStatus == Connecting) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": Network is connecting. Add to queue";
        m_locationQueue.enqueue(locationData);
        return;
    }

    if (m_currentLocationUpdate == locationData) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": restart locationData update with new search string";
        m_searchStringChanged = true;
        m_locationsFutureWatcher->cancel();
        return;
    }

    // if busy then some weather data already updating. Just locationData to queue.
    if (m_isBusy) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": control is busy. Add locationData to queue";
        m_locationQueue.enqueue(locationData);
        return;
    }

    qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": start search for " << locationData->searchString();
    m_isBusy = true;

    m_currentLocationUpdate = locationData;

    if (m_fetchThread && !m_fetchThread->isRunning()) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": start fetch thread";
        m_fetchThread->start();
    }

    if (!m_locationsFutureWatcher) {
        m_locationsFutureWatcher = std::make_shared<QFutureWatcher<std::shared_ptr<Locations>>>();
        connect(m_locationsFutureWatcher.get(), &QFutureWatcher<std::shared_ptr<Locations>>::finished, this, &IonControl::onFetchLocationsEnded);
        connect(m_locationsFutureWatcher.get(), &QFutureWatcher<std::shared_ptr<Locations>>::canceled, this, &IonControl::onFetchLocationsCancelled);
    }

    const auto promise = std::make_shared<QPromise<std::shared_ptr<Locations>>>();

    m_locationsFutureWatcher->setFuture(promise->future());

    Q_EMIT fetchLocationsRequest(promise, m_currentLocationUpdate->searchString());
}

void IonControl::onFetchLocationsEnded()
{
    if (m_currentLocationUpdate && m_currentLocationUpdate.use_count() > 1) {
        if (m_locationsFutureWatcher->future().resultCount()) {
            qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName
                                         << ": fetch locations ended for search string:" << m_currentLocationUpdate->searchString();
            m_currentLocationUpdate->setLocations(m_locationsFutureWatcher->result());
        } else {
            auto errorLocations = std::make_shared<Locations>();
            errorLocations->setError();
            qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName
                                         << ": fetch locations error for search string:" << m_currentLocationUpdate->searchString();
            m_currentLocationUpdate->setLocations(errorLocations);
        }
    }

    m_currentLocationUpdate = nullptr;

    m_isBusy = false;

    const auto instance = QNetworkInformation::instance();
    if (instance && instance->reachability() == QNetworkInformation::Reachability::Online) {
        checkQueues();
    }
}

void IonControl::onFetchLocationsCancelled()
{
    if (m_currentLocationUpdate && m_currentLocationUpdate.use_count() > 1) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": fetch locations cancelled";
        if (m_searchStringChanged) {
            qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": restart search for searchstring: " << m_currentLocationUpdate->searchString();
            m_locationQueue.enqueue(m_currentLocationUpdate);
            m_searchStringChanged = false;
        }
    }

    m_currentLocationUpdate = nullptr;

    m_isBusy = false;

    const auto instance = QNetworkInformation::instance();
    if (instance && instance->reachability() == QNetworkInformation::Reachability::Online) {
        checkQueues();
    }
}

void IonControl::updateForecast(const std::shared_ptr<ForecastData> &forecastData)
{
    qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": get forecast request";

    if (m_forecastQueue.contains(forecastData)) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": forecastData already in queue. Don't add another one";
        return;
    }

    // if provider is unavailable save the forecastData and try to connect to provider again.
    if (m_connectionStatus == ProviderUnavailable) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": Provider is unreachable. Skip update forecast";
        m_forecastQueue.enqueue(forecastData);
        checkProviderAvailability();
        return;
    }

    // skip update and add forecast to the queue if the network is offline
    if (m_connectionStatus == Disconnected) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": Network connection is unavailable. Skip update forecast";
        auto errorForecast = std::make_shared<Forecast>();
        errorForecast->setError();
        forecastData->setForecast(errorForecast);
        m_forecastQueue.enqueue(forecastData);
        return;
    }

    if (m_connectionStatus == Connecting) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": Network is connecting. Add to queue";
        m_forecastQueue.enqueue(forecastData);
        return;
    }

    if (m_currentForecastUpdate == forecastData) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": forecastData is already being updated. Don't add another one to queue";
        return;
    }

    // if busy then some weather data already updating. Just forecastData to queue.
    if (m_isBusy) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": control is busy. Add forecastData to queue";
        m_forecastQueue.enqueue(forecastData);
        return;
    }
    qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": start update for " << forecastData->placeInfo();
    m_isBusy = true;

    m_currentForecastUpdate = forecastData;

    if (m_fetchThread && !m_fetchThread->isRunning()) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": start fetch thread: " << m_fetchThread.get();
        m_fetchThread->start();
    }

    if (!m_forecastFutureWatcher) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": create new location watcher";
        m_forecastFutureWatcher = std::make_shared<QFutureWatcher<std::shared_ptr<Forecast>>>();
        connect(m_forecastFutureWatcher.get(), &QFutureWatcher<std::shared_ptr<Forecast>>::finished, this, &IonControl::onFetchForecastEnded);
        connect(m_forecastFutureWatcher.get(), &QFutureWatcher<std::shared_ptr<Forecast>>::canceled, this, &IonControl::onFetchForecastCancelled);
    }

    const auto promise = std::make_shared<QPromise<std::shared_ptr<Forecast>>>();

    m_forecastFutureWatcher->setFuture(promise->future());

    Q_EMIT fetchForecastRequest(promise, m_currentForecastUpdate->placeInfo());
}

void IonControl::onFetchForecastEnded()
{
    if (m_currentForecastUpdate && m_currentForecastUpdate.use_count() > 1) {
        if (m_forecastFutureWatcher->future().resultCount()) {
            qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": fetch weather ended for place:" << m_currentForecastUpdate->placeInfo();
            m_currentForecastUpdate->setForecast(m_forecastFutureWatcher->result());
        } else {
            auto errorForecast = std::make_shared<Forecast>();
            qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": fetch weather error for place:" << m_currentForecastUpdate->placeInfo();
            errorForecast->setError();
            m_currentForecastUpdate->setForecast(errorForecast);
        }
    }

    m_currentForecastUpdate = nullptr;

    m_isBusy = false;

    const auto instance = QNetworkInformation::instance();
    if (instance && instance->reachability() == QNetworkInformation::Reachability::Online) {
        checkQueues();
    }
}

void IonControl::onFetchForecastCancelled()
{
    qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": fetch forecast cancelled for place:" << m_currentForecastUpdate->placeInfo();

    m_currentForecastUpdate = nullptr;

    m_isBusy = false;

    const auto instance = QNetworkInformation::instance();
    if (instance && instance->reachability() == QNetworkInformation::Reachability::Online) {
        checkQueues();
    }
}

void IonControl::checkQueues()
{
    // check location queue first because locations mostly used when configuring. We don't want the user wait until all forecasts will be updated
    while (!m_locationQueue.isEmpty()) {
        const auto location = m_locationQueue.dequeue();
        if (location.use_count() != 1) {
            qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": location queue is not empty. Start update next locationData";
            updateLocations(location);
            return;
        }
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": locationData is not used anymore. Skipping";
    }

    m_locationsFutureWatcher.reset();

    while (!m_forecastQueue.isEmpty()) {
        const auto forecast = m_forecastQueue.dequeue();
        if (forecast.use_count() != 1) {
            qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": forecast queue is not empty. Start update next forecastData";
            updateForecast(forecast);
            return;
        }
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": forecastData is not anymore. Skipping";
    }

    m_forecastFutureWatcher.reset();

    if (m_fetchThread && m_fetchThread->isRunning()) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": stop fetch thread: " << m_fetchThread.get();
        m_fetchThread->quit();
    }
}

void IonControl::checkProviderAvailability()
{
    qCDebug(WEATHER::CONTROLLER) << "ionControl " << m_ionName << ": check provider availability";
    QNetworkRequest request(m_ion->providerURL());

    QNetworkReply *reply = m_manager->head(request); // HEAD is fast, no body

    connect(reply, &QNetworkReply::finished, [reply, this]() {
        // We only need to check whether we can connect to the provider,
        // so ContentNotFoundError is not a problem here because the server
        // was successfully reached.
        if (reply->error() == QNetworkReply::NoError || reply->error() == QNetworkReply::ContentNotFoundError) {
            qCDebug(WEATHER::CONTROLLER) << "ionControl " << m_ionName << ": provider is online. Number of reconnecting attempts: " << m_reconnectAttempts;
            m_connectionStatus = Connected;
            m_reconnectAttempts = 0;
            onProviderAvailabilityChanged();
        } else {
            ++m_reconnectAttempts;
            if (m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
                m_providerReconnectTimer->start();
            } else {
                qCDebug(WEATHER::CONTROLLER) << "ionControl " << m_ionName << ": can't connect to provider";
                m_connectionStatus = ProviderUnavailable;
                m_reconnectAttempts = 0;
                onProviderAvailabilityChanged();
            }
        }
        reply->deleteLater();
    });
}

void IonControl::onOnlineStateChanged(QNetworkInformation::Reachability reachability)
{
    // if network is became online start update by check queue. If offline then cancel update and save all not updated data
    if (reachability == QNetworkInformation::Reachability::Online) {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": Network is online. Starting updating weather";
        m_connectionStatus = Connecting;
        m_networkReconnectTimer->start();
    } else {
        qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": Network is offline. Stop updating weather";
        m_connectionStatus = Disconnected;

        m_networkReconnectTimer->stop();
        m_providerReconnectTimer->stop();

        if (m_fetchThread && m_fetchThread->isRunning()) {
            qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": stop fetch thread: " << m_fetchThread.get();
            m_fetchThread->quit();
        }

        if (m_currentLocationUpdate) {
            qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": save location: " << m_currentLocationUpdate->searchString();
            m_locationQueue.enqueue(m_currentLocationUpdate);
        }
        if (m_currentForecastUpdate) {
            qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": save forecast: " << m_currentForecastUpdate->placeInfo();
            m_forecastQueue.enqueue(m_currentForecastUpdate);
        }
        m_currentForecastUpdate.reset();
        m_currentLocationUpdate.reset();
    }
}

void IonControl::onProviderAvailabilityChanged()
{
    if (m_connectionStatus == Connected) {
        checkQueues();
    } else if (m_connectionStatus == ProviderUnavailable) {
        // if after all attempts provider is unavailable, return errors for all requests
        for (auto it = m_locationQueue.begin(); it != m_locationQueue.end(); ++it) {
            auto locationsData = std::make_shared<Locations>();
            locationsData->setError();
            (*it)->setLocations(locationsData);
        }
        m_locationQueue.clear();

        for (auto it = m_forecastQueue.begin(); it != m_forecastQueue.end(); ++it) {
            qCDebug(WEATHER::CONTROLLER) << "IonControl " << m_ionName << ": save forecast: " << m_currentForecastUpdate->placeInfo();
            auto forecastData = std::make_shared<Forecast>();
            forecastData->setError();
            (*it)->setForecast(forecastData);
        }
        m_forecastQueue.clear();
    }
}
