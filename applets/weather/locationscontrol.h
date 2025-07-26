/*
 * SPDX-FileCopyrightText: 2016 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "locationsdata.h"

#include <QConcatenateTablesProxyModel>
#include <QList>

#include <qqmlintegration.h>

class WeatherDataMonitor;

/*!
 * \class LocationsControl
 *
 * \brief Request locations from different providers and concatenate them in one table model
 *
 * The LocationsControl class is used from qml to search locations from different providers which
 * then is concatenated into one table model.
 */
class LocationsControl : public QConcatenateTablesProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool hasProviders READ getHasProviders CONSTANT)

public:
    explicit LocationsControl(QObject *parent = nullptr);
    ~LocationsControl() override;

    /*!
     * request search for locations with searchString
     */
    Q_INVOKABLE void searchLocations(const QString &searchString);
    /*!
     * Used to clear all currently available locations. Implicitly called
     * before each new search.
     */
    Q_INVOKABLE void clear();

    /*!
     * return the display name of a provider.
     */
    Q_INVOKABLE QString getProviderDisplayName(const QString &providerName) const;

    /*!
     * return provider credit related to location at \c position.
     */
    Q_INVOKABLE QVariant getProviderCreditByIndex(int position);
    /*!
     * return place display name related to location at \c position.
     */
    Q_INVOKABLE QVariant getPlaceDisplayNameByIndex(int position);
    /*!
     * return provider's name related to location at \c position.
     */
    Q_INVOKABLE QVariant getProviderByIndex(int position);
    /*!
     * return placeInfo related to location at \c position. It then can
     * be used to retrieve forecast by this info.
     */
    Q_INVOKABLE QVariant getPlaceInfoByIndex(int position);

private:
    bool getHasProviders() const;

    /*!
     * Used whenever high quality providers return empty locations to check
     * low quality providers.
     */
    void checkFallbackLocations();
    /*!
     * Called after each locations update from the providers. Monitors number
     * of remained providers which update their locations and emits \c locationSearchDone
     * signal when all providers updated their locations.
     */
    void completeSearch();

Q_SIGNALS:
    /*!
     * Emitted when search for locations ended
     */
    void locationSearchDone(bool success, const QString &searchString);

private Q_SLOTS:
    /*!
     * Slot which is called whenever search have ended for a provider
     */
    void onSearchEnded(const QString &provider);

private:
    QHash<QString, std::shared_ptr<LocationsData>> m_locationsData;
    QHash<QString, std::shared_ptr<LocationsData>> m_fallbackLocationsData;

    QList<std::shared_ptr<LocationsData>> m_locationsOrder;

    std::shared_ptr<WeatherDataMonitor> m_weatherDataMonitor;

    bool m_useFallbackLocations;
    QString m_searchString;
    int m_checkedInCount;
};
