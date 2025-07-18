/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "station.h"

Station::Station()
    : m_isDataPresent(false)
{
}

Station::~Station()
{
}

bool Station::isDataPresent() const
{
    return m_isDataPresent;
}

QVariant Station::getStation() const
{
    if (m_station.has_value()) {
        return *m_station;
    }
    return {};
}

QVariant Station::getPlace() const
{
    if (m_place.has_value()) {
        return *m_place;
    }
    return {};
}

QVariant Station::getRegion() const
{
    if (m_region.has_value()) {
        return *m_region;
    }
    return {};
}

QVariant Station::getCountry() const
{
    if (m_country.has_value()) {
        return *m_country;
    }
    return {};
}

QVariant Station::getLatitude() const
{
    if (m_latitude.has_value()) {
        return *m_latitude;
    }
    return {};
}

QVariant Station::getLongitude() const
{
    if (m_longitude.has_value()) {
        return *m_longitude;
    }
    return {};
}

void Station::setStation(const QString &station)
{
    m_station = station;
    m_isDataPresent = true;
}

void Station::setPlace(const QString &place)
{
    m_place = place;
    m_isDataPresent = true;
}

void Station::setRegion(const QString &region)
{
    m_region = region;
    m_isDataPresent = true;
}

void Station::setCountry(const QString &country)
{
    m_country = country;
    m_isDataPresent = true;
}

void Station::setCoordinates(qreal latitude, qreal longitude)
{
    m_latitude = latitude;
    m_longitude = longitude;
    m_isDataPresent = true;
}

#include "moc_station.cpp"
