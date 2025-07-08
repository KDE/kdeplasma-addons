/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "locations.h"

Locations::Locations(QObject *parent)
    : QAbstractListModel(parent)
    , m_error(false)
{
}

Locations::~Locations()
{
}

bool Locations::isError() const
{
    return m_error;
}

void Locations::setError()
{
    m_error = true;
}

void Locations::setCredit(const QString &credit)
{
    m_credit = credit;
}

int Locations::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_locations.size();
}

QVariant Locations::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (role == Credit) {
        return m_credit;
    }

    const auto &location = m_locations.at(index.row());

    switch (role) {
    case PlaceInfo:
        return location.placeInfo().has_value() ? *location.placeInfo() : QVariant();
    case DisplayName:
        return location.displayName().has_value() ? *location.displayName() : QVariant();
    case Code:
        return location.code().has_value() ? *location.code() : QVariant();
    case Station:
        return location.station().has_value() ? *location.station() : QVariant();
    case Latitude:
        return location.coordinates().has_value() ? location.coordinates()->x() : QVariant();
    case Longitude:
        return location.coordinates().has_value() ? location.coordinates()->y() : QVariant();
    }

    return {};
}

QHash<int, QByteArray> Locations::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Credit] = "credit";
    roles[PlaceInfo] = "placeInfo";
    roles[DisplayName] = "displayName";
    roles[Code] = "code";
    roles[Station] = "station";
    roles[Latitude] = "latitude";
    roles[Longitude] = "longitude";

    return roles;
}

void Locations::addLocation(const Location &location)
{
    beginInsertRows(QModelIndex(), m_locations.size(), m_locations.size());
    if (!location.placeInfo().has_value() || !location.station().has_value()) {
        return;
    }
    m_locations.append(location);
    endInsertRows();
}

void Locations::addLocations(const QList<Location> &locations)
{
    beginInsertRows(QModelIndex(), m_locations.size(), locations.size());
    for (const Location &location : locations) {
        if (!location.placeInfo().has_value() || !location.station().has_value()) {
            return;
        }
        m_locations.append(location);
    }
    endInsertRows();
}

Location::Location()
{
}

Location::~Location()
{
}

const std::optional<QString> &Location::placeInfo() const
{
    return m_placeInfo;
}

const std::optional<QString> &Location::displayName() const
{
    return m_displayName;
}

const std::optional<QString> &Location::code() const
{
    return m_code;
}

const std::optional<QString> &Location::station() const
{
    return m_station;
}

const std::optional<QPointF> &Location::coordinates() const
{
    return m_coordinates;
}

void Location::setPlaceInfo(const QString &placeInfo)
{
    m_placeInfo = placeInfo;
}

void Location::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

void Location::setCode(const QString &code)
{
    m_code = code;
}

void Location::setStation(const QString &station)
{
    m_station = station;
}

void Location::setCoordinates(const QPointF &location)
{
    m_coordinates = location;
}

#include "moc_locations.cpp"
