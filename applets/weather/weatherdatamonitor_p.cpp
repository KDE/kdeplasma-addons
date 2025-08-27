/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "weatherdatamonitor_p.h"

#include "weathercontroller_debug.h"

#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QPluginLoader>

#include <KPluginFactory>
#include <KPluginMetaData>

#include "ioncontrol_p.h"

using namespace Qt::StringLiterals;

std::shared_ptr<WeatherDataMonitor> WeatherDataMonitor::instance()
{
    static std::weak_ptr<WeatherDataMonitor> s_clip;
    if (s_clip.expired()) {
        std::shared_ptr<WeatherDataMonitor> ptr{new WeatherDataMonitor};
        s_clip = ptr;
        return ptr;
    }
    return s_clip.lock();
}

WeatherDataMonitor::WeatherDataMonitor(QObject *parent)
    : QObject(parent)
{
    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: start initializing";

    const auto plugins = KPluginMetaData::findPlugins("plasma/weather_ions", {}, KPluginMetaData::AllowEmptyMetaData);

    for (const auto &plugin : plugins) {
        m_providers << plugin.pluginId();
        m_ions.insert(plugin.pluginId(), plugin);
        qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: plugin " << plugin.pluginId() << " added";
    }

    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: initialized";
}

WeatherDataMonitor::~WeatherDataMonitor()
{
    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: destroyed";
}

QStringList WeatherDataMonitor::getProviders() const
{
    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: get providers request";
    return m_providers;
}

int WeatherDataMonitor::providerQuality(const QString &providerName) const
{
    if (auto it = m_ions.constFind(providerName); it != m_ions.cend()) {
        const QJsonObject &metaData = it.value().rawData();
        auto qualityIt = metaData.constFind("Quality"_L1);
        if (qualityIt == metaData.constEnd()) {
            qCWarning(WEATHER::CONTROLLER) << "WeatherDataMonitor: quality not found for plugin: " << it.key();
            return -2;
        }
        return qualityIt->toInt();
    }
    return -2;
}

QString WeatherDataMonitor::providerDisplayName(const QString &providerName) const
{
    if (auto it = m_ions.constFind(providerName); it != m_ions.cend()) {
        return it->name();
    }
    return {};
}

std::shared_ptr<LocationsData> WeatherDataMonitor::getLocationData(const QString &providerName)
{
    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: get location request for provider: " << providerName;

    // If there is no IonInfo with such name then create a new
    if (auto it = m_ionControls.find(providerName); it == m_ionControls.end()) {
        if (!createIonControl(providerName)) {
            qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: can't find provider: " << providerName;
            return nullptr;
        }
    }

    std::shared_ptr<LocationsData> locationData{new LocationsData(providerName)};

    ++m_ionControls[providerName].locationCount;

    connect(locationData.get(), &LocationsData::locationDataRemoved, this, &WeatherDataMonitor::onLocationDataRemoved);

    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: LocationData successfully created. Return locationData";
    return locationData;
}

std::shared_ptr<ForecastData> WeatherDataMonitor::getForecastData(const QString &providerName, const QString &placeInfo)
{
    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: get forecast request for provider: " << providerName;

    if (auto it = m_ionControls.find(providerName); it != m_ionControls.end()) {
        // If such IonInfo exists then check it ForecastDataList for ForecastData with similar placeInfo.
        for (auto listIt = it.value().forecastDataList.begin(); listIt != it.value().forecastDataList.end(); ++listIt) {
            if (listIt->first == placeInfo) {
                // If ForecastData with such placeInfo exists then return pointer to it.
                qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: forecast with provider: " << providerName << " and place: " << placeInfo
                                             << " already exists. Return it";
                return listIt->second.lock();
            }
        }
    } else {
        // If there is no IonInfo with such name then create a new.
        if (!createIonControl(providerName)) {
            qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: can't find provider: " << providerName;
            return nullptr;
        }
    }

    // Create new ForecastData and add it to ForecastDataList
    std::shared_ptr<ForecastData> forecastData{new ForecastData(providerName, placeInfo)};
    connect(forecastData.get(), &ForecastData::forecastDataRemoved, this, &WeatherDataMonitor::onWeatherDataRemoved);

    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: add new forecast to forecast list";
    m_ionControls[providerName].forecastDataList.append({placeInfo, forecastData});

    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: ForecastData successfully created. Return";
    return forecastData;
}

void WeatherDataMonitor::updateForecastData(const std::shared_ptr<ForecastData> &forecastData)
{
    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: update forecastData request";
    if (auto it = m_ionControls.find(forecastData->provider()); it != m_ionControls.end()) {
        it->ionControl->updateForecast(forecastData);
        qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: update forecastData successful";
        return;
    }
    qCWarning(WEATHER::CONTROLLER) << "WeatherDataMonitor: forecastData don't exist";
}

void WeatherDataMonitor::updateLocationData(const std::shared_ptr<LocationsData> &locationData, const QString &searchString)
{
    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: update locationData request";
    if (auto it = m_ionControls.find(locationData->provider()); it != m_ionControls.end()) {
        qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: update locationData successful";
        // Update searchString of LocationData. Only WeatherDataMonitor can update searchString which prevents situations where
        // LocationData contains unrelated locations and searchString.
        locationData->setSearchString(searchString);
        it->ionControl->updateLocations(locationData);
        return;
    }
    qCWarning(WEATHER::CONTROLLER) << "WeatherDataMonitor: locationData don't exist";
}

void WeatherDataMonitor::onWeatherDataRemoved(const QString &provider, const QString &place)
{
    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: forecastData remove signal arrived for provider: " << provider << " place: " << place;
    if (auto it = m_ionControls.find(provider); it != m_ionControls.end()) {
        for (auto listIt = it->forecastDataList.begin(); listIt != it->forecastDataList.end(); ++listIt) {
            if (listIt->second.expired() && listIt->first == place) {
                qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: remove forecast data from forecast data list. List size: " << it->forecastDataList.size();
                it->forecastDataList.erase(listIt);
                break;
            }
        }
        finishRemoving(provider);
    }
}

void WeatherDataMonitor::onLocationDataRemoved(const QString &provider)
{
    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: locationsData remove signal arrived for provider: " << provider;
    if (auto it = m_ionControls.find(provider); it != m_ionControls.end()) {
        --it->locationCount;
        qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: update locations counter: " << it->locationCount;
        finishRemoving(provider);
    }
}

void WeatherDataMonitor::finishRemoving(const QString &providerName)
{
    if (auto it = m_ionControls.find(providerName); it != m_ionControls.end()) {
        qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: update check plugin for provider: " << providerName;

        if (it->locationCount != 0 || !it->forecastDataList.isEmpty()) {
            qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: ion is in use. Skip removing";
            return;
        }

        qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: ion controller is not needed any more. Remove it";

        // If ionInfo isn't used by LocationData and ForecastData then remove it
        m_ionControls.erase(it);
    }
}

bool WeatherDataMonitor::createIonControl(const QString &providerName)
{
    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: ion control is not exists. Create new";

    auto it = m_ions.find(providerName);

    if (it == m_ions.end()) {
        qCWarning(WEATHER::CONTROLLER) << "WeatherDataMonitor: ion loader " << providerName << " not found";
        return false;
    }

    auto result = KPluginFactory::instantiatePlugin<Ion>(it.value());

    if (!result) {
        qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: failed to retrieve ion";
        return false;
    }
    std::shared_ptr<Ion> ion(result.plugin);

    std::shared_ptr<QThread> workerThread{new QThread()};

    // Plugin is moved to another thread to isolate potentially blocking operations
    if (!ion->moveToThread(workerThread.get())) {
        qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: failed to move to thread ion: " << providerName;
        return false;
    }

    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: Ion moved to thread: " << ion->thread();

    const IonInfo info{
        .ionControl = std::shared_ptr<IonControl>{new IonControl(providerName, ion, workerThread)},
    };
    m_ionControls.insert(providerName, info);
    qCDebug(WEATHER::CONTROLLER) << "WeatherDataMonitor: successfully created";

    return true;
}

#include "moc_weatherdatamonitor_p.cpp"
