/*
 * SPDX-FileCopyrightText: 2009 Petri Damstén <damu@iki.fi>
 * SPDX-FileCopyrightText: 2016, 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "locationscontrol.h"

#include "weathercontroller_debug.h"

#include "weatherdatamonitor_p.h"

LocationsControl::LocationsControl(QObject *parent)
    : QConcatenateTablesProxyModel(parent)
    , m_useFallbackLocations(false)
    , m_checkedInCount(0)
{
    m_weatherDataMonitor = WeatherDataMonitor::instance();

    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: successfully initialized";
}

LocationsControl::~LocationsControl()
{
    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: destroying";
}

void LocationsControl::searchLocations(const QString &searchString)
{
    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: search locations request for search string: " << searchString;
    clear();

    m_searchString = searchString;

    if (searchString.isEmpty()) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: search string is empty. Skip searching";
        return;
    }

    // if this is the first search then initialize the high quality providers
    if (m_locationsData.isEmpty()) {
        QStringList providers = m_weatherDataMonitor->getProviders();

        for (const auto &provider : providers) {
            if (m_weatherDataMonitor->providerQuality(provider) < 0) {
                qCDebug(WEATHER::CONTROLLER) << "LocationsControl: skip provider: " << provider;
                continue;
            }

            const auto &locationsData = m_weatherDataMonitor->getLocationData(provider);

            if (!locationsData) {
                qCDebug(WEATHER::CONTROLLER) << "LocationsControl: locationData is not valid. Skipping";
                continue;
            }

            connect(locationsData.get(), &LocationsData::locationsUpdated, this, &LocationsControl::onSearchEnded);

            m_locationsData.insert(provider, locationsData);
        }
    }

    // if high quality providers are not present use low quality instead
    if (m_locationsData.isEmpty()) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: high quality providers are empty";
        checkFallbackLocations();
        return;
    }

    for (const auto &locationData : m_locationsData) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: search location: " << searchString << " by provider: " << locationData->provider();
        m_weatherDataMonitor->updateLocationData(locationData, m_searchString);
        ++m_checkedInCount;
    }
}

void LocationsControl::onSearchEnded(const QString &provider)
{
    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: search ended signal arrived for provider: " << provider;
    std::shared_ptr<LocationsData> locationsData;

    // if we used fallback locations then search in fallback locations hash
    if (!m_useFallbackLocations) {
        if (auto it = m_locationsData.find(provider); it != m_locationsData.end()) {
            locationsData = *it;
        }
    } else {
        if (auto it = m_fallbackLocationsData.find(provider); it != m_fallbackLocationsData.end()) {
            locationsData = *it;
        }
    }

    if (!locationsData) {
        qCWarning(WEATHER::CONTROLLER) << "LocationsControl: locationData is not valid";
        return;
    }

    if (!locationsData->locations()) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: locationData does not contain locations";
        completeSearch();
        return;
    }

    if (locationsData->locations()->isError()) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: locationData returned error";
        completeSearch();
        return;
    }

    if (locationsData->locations()->rowCount() == 0) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: locationData returned empty locations. Skipping";
        completeSearch();
        return;
    }

    // Insert the arrived result at its sorted position so the list stays in a
    // deterministic order regardless of which provider responded first.
    // Primary key: provider quality (desc). Tie-breaker: provider name (asc).
    auto shouldComeBefore = [this](const std::shared_ptr<LocationsData> &a, const std::shared_ptr<LocationsData> &b) {
        const int qa = m_weatherDataMonitor->providerQuality(a->provider());
        const int qb = m_weatherDataMonitor->providerQuality(b->provider());
        if (qa != qb) {
            return qa > qb;
        }
        return a->provider() < b->provider();
    };

    int insertPosition = 0;
    while (insertPosition < m_locationsOrder.size() && shouldComeBefore(m_locationsOrder[insertPosition], locationsData)) {
        ++insertPosition;
    }

    // Fast path: the new result belongs at the end, no need to touch existing models.
    if (insertPosition == m_locationsOrder.size()) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: append " << locationsData->locations()->rowCount() << " locations";
        m_locationsOrder.append(locationsData);
        addSourceModel(locationsData->locations().get());
        completeSearch();
        return;
    }

    // QConcatenateTablesProxyModel can only append, so pop the trailing models,
    // insert the new one, then re-append them in order.
    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: insert " << locationsData->locations()->rowCount() << " locations at position " << insertPosition;

    for (int i = insertPosition; i < m_locationsOrder.size(); ++i) {
        removeSourceModel(m_locationsOrder[i]->locations().get());
    }

    m_locationsOrder.insert(insertPosition, locationsData);

    for (int i = insertPosition; i < m_locationsOrder.size(); ++i) {
        addSourceModel(m_locationsOrder[i]->locations().get());
    }

    completeSearch();
}

void LocationsControl::completeSearch()
{
    --m_checkedInCount;

    if (m_checkedInCount) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: Waiting for another " << m_checkedInCount << " providers";
        return;
    }

    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: Locations search ended";

    // If the high quality providers have not returned any locations then request from the low quality providers
    if (!m_useFallbackLocations && m_locationsOrder.isEmpty()) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: search ended with empty locations";
        checkFallbackLocations();
        return;
    }

    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: Location search ended. The number of locations: " << rowCount();

    Q_EMIT locationSearchDone(!m_locationsData.empty(), m_searchString);
}

void LocationsControl::checkFallbackLocations()
{
    m_useFallbackLocations = true;
    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: Location model is empty. Use fallback locations";
    // if this is the first search for low quality providers then initialize them
    if (m_fallbackLocationsData.isEmpty()) {
        for (const auto &provider : m_weatherDataMonitor->getProviders()) {
            if (m_weatherDataMonitor->providerQuality(provider) >= 0) {
                continue;
            }

            qCDebug(WEATHER::CONTROLLER) << "LocationsControl: add new provider for fallback locations: " << provider;
            auto locationsData = m_weatherDataMonitor->getLocationData(provider);
            if (!locationsData) {
                qCDebug(WEATHER::CONTROLLER) << "LocationsControl: fallback locationData is not valid. Skipping";
                continue;
            }

            connect(locationsData.get(), &LocationsData::locationsUpdated, this, &LocationsControl::onSearchEnded);
            m_fallbackLocationsData.insert(provider, locationsData);
        }
    }

    for (const auto &locationData : m_fallbackLocationsData) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: search for provider: " << locationData->provider();
        WeatherDataMonitor::instance()->updateLocationData(locationData, m_searchString);
        ++m_checkedInCount;
    }

    // If fallback providers not present finish search
    if (m_checkedInCount == 0) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: fallback providers empty. Finish search";
        Q_EMIT locationSearchDone(!m_locationsOrder.empty(), m_searchString);
    }
}

void LocationsControl::clear()
{
    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: clearing";
    m_checkedInCount = 0;
    m_useFallbackLocations = false;
    beginResetModel();
    for (const auto &locationData : m_locationsOrder) {
        removeSourceModel(locationData->locations().get());
    }
    endResetModel();
    m_locationsOrder.clear();
    m_searchString.clear();
}

QString LocationsControl::getProviderDisplayName(const QString &providerName) const
{
    return m_weatherDataMonitor->providerDisplayName(providerName);
}

QVariant LocationsControl::getPlaceDisplayNameByIndex(int position)
{
    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: get placeInfo by index: " << position;
    QModelIndex locationPosition = index(position, 0, QModelIndex());
    QVariant placeName = data(locationPosition, Locations::DisplayName);
    if (placeName.isValid()) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: found placeName: " << placeName.toString();
        return placeName;
    }
    placeName = data(locationPosition, Locations::Station);
    if (placeName.isValid()) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: found Station: " << placeName.toString();
        return placeName;
    }
    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: Can't find any names";
    return {};
}

QVariant LocationsControl::getProviderByIndex(int position)
{
    if (position >= rowCount()) {
        return {};
    }

    int locationCount = 0;

    for (const auto &locationsData : m_locationsOrder) {
        if (locationCount + locationsData->locations()->rowCount() > position) {
            return locationsData->provider();
        }
        locationCount += locationsData->locations()->rowCount();
    }

    return {};
}

QVariant LocationsControl::getPlaceInfoByIndex(int position)
{
    QModelIndex locationPosition = index(position, 0, QModelIndex());
    return data(locationPosition, Locations::PlaceInfo);
}

bool LocationsControl::getHasProviders() const
{
    return !m_weatherDataMonitor->getProviders().isEmpty();
}

#include "moc_locationscontrol.cpp"
