/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QObject>

#include "locations.h"

/*!
 * \class LocationsData
 *
 * \brief Manages locations
 *
 * Used to manage locations, provide additional info about locations and emit
 * \c locationSearchEnded signal whenever locations is updated. Created only by
 * \c WeatherDataMonitor.
 */
class LocationsData : public QObject
{
    Q_OBJECT

    friend class WeatherDataMonitor;
    friend class IonControl;

public:
    /*!
     * Return search string by which locations is searched
     */
    QString searchString() const;

    /*!
     * Return provider for which this LocationsData is related
     */
    QString provider() const;

    /*!
     * Return the locations. If the locations is empty return \c nullptr
     */
    const std::shared_ptr<Locations> &locations() const;

    ~LocationsData() override;

private:
    explicit LocationsData(const QString &provider);

    /*!
     * Set the searchString. Used only by \c WeatherDataMonitor.
     */
    void setSearchString(const QString &searchString);

    /*!
     * Set the locations. Used only by \c IonControl.
     */
    void setLocations(const std::shared_ptr<Locations> &locations);

Q_SIGNALS:
    /*!
     * Emitted whenever the locations are updated.
     */
    void locationsUpdated(const QString &provider);

    /*!
     * Emitted when the LocationsData is about to be removed
     */
    void locationDataRemoved(const QString &provider);

private:
    QString m_searchString;

    QString m_provider;

    int m_quality;

    std::shared_ptr<Locations> m_locations;
};
