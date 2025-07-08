/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "locationsdata.h"

#include "weathercontroller_debug.h"

LocationsData::LocationsData(const QString &provider)
    : m_provider(provider)
{
}

LocationsData::~LocationsData()
{
    Q_EMIT locationDataRemoved(m_provider);
}

QString LocationsData::searchString() const
{
    return m_searchString;
}

QString LocationsData::provider() const
{
    return m_provider;
}

void LocationsData::setSearchString(const QString &searchString)
{
    m_searchString = searchString;
}

const std::shared_ptr<Locations> &LocationsData::locations() const
{
    return m_locations;
}

void LocationsData::setLocations(const std::shared_ptr<Locations> &locations)
{
    m_locations = locations;
    Q_EMIT locationsUpdated(m_provider);
}
