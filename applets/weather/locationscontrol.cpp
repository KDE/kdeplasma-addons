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

    // if there are no locations in model or the quality of the last location data is better than arrived we just add the
    // arrived locations to the end of the model
    if (m_locationsOrder.isEmpty()
        || m_weatherDataMonitor->providerQuality(locationsData->provider()) <= m_weatherDataMonitor->providerQuality(m_locationsOrder.last()->provider())) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: append " << locationsData->locations()->rowCount() << " locations";
        m_locationsOrder.append(locationsData);
        addSourceModel(locationsData->locations().get());
        completeSearch();
        return;
    }

    // Sort all locations by quality. Locations with better quality is above others
    for (int qualityPosition = 0; qualityPosition < m_locationsOrder.size(); ++qualityPosition) {
        if (m_weatherDataMonitor->providerQuality(locationsData->provider())
            > m_weatherDataMonitor->providerQuality(m_locationsOrder[qualityPosition]->provider())) {
            qCDebug(WEATHER::CONTROLLER) << "LocationsControl: add model with " << locationsData->locations()->rowCount(QModelIndex())
                                         << " locations, begin at position " << qualityPosition;

            // Remove all locations from the model with quality less than the quality of arrived provider,
            // insert provider and re-add the locations with less quality because QConcatenateTablesProxyModel
            // don't support adding at arbitrary position
            QList<std::shared_ptr<LocationsData>> tempLocationsOrder;

            for (int removePosition = qualityPosition; removePosition < m_locationsOrder.size(); ++removePosition) {
                auto worstQualityData = m_locationsOrder[removePosition];
                removeSourceModel(worstQualityData->locations().get());
                tempLocationsOrder.append(worstQualityData);
            }

            addSourceModel(locationsData->locations().get());
            m_locationsOrder.insert(qualityPosition, locationsData);

            for (const auto &worstQualityData : tempLocationsOrder) {
                addSourceModel(worstQualityData->locations().get());
            }
            break;
        }
    }

    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: add locations from provider: " << provider << " ended";

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

QVariant LocationsControl::getProviderCreditByIndex(int position)
{
    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: get placeInfo by index: " << position;
    QModelIndex locationPosition = index(position, 0, QModelIndex());
    return data(locationPosition, Locations::Credit);
}

QVariant LocationsControl::getProviderByIndex(int position)
{
    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: get provider by index: " << position;
    if (position >= rowCount()) {
        qCDebug(WEATHER::CONTROLLER) << "LocationsControl: index is not valid";
        return {};
    }

    int locationCount = 0;

    for (const auto &locationsData : m_locationsOrder) {
        if (locationCount + locationsData->locations()->rowCount() > position) {
            qCDebug(WEATHER::CONTROLLER) << "LocationsControl: return provider: " << locationsData->provider();
            return locationsData->provider();
        }
        locationCount += locationsData->locations()->rowCount();
    }

    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: placeInfo have not been found";

    return {};
}

QVariant LocationsControl::getPlaceInfoByIndex(int position)
{
    qCDebug(WEATHER::CONTROLLER) << "LocationsControl: get placeInfo by index: " << position;
    QModelIndex locationPosition = index(position, 0, QModelIndex());
    return data(locationPosition, Locations::PlaceInfo);
}

bool LocationsControl::getHasProviders() const
{
    return !m_weatherDataMonitor->getProviders().isEmpty();
}

#include "moc_locationscontrol.cpp"
