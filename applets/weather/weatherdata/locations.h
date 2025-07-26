/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <weatherdata_export.h>

#include <QAbstractListModel>
#include <QList>
#include <QPointF>

#include <qqmlintegration.h>

class Location;

/*!
 * \class Station
 *
 * \brief Data about the weather station
 *
 * Data about the weather station:
 * "placeInfo": string, specific string used by provider to receive the forecast, required
 * "displayName": string, display name of the weather station, optional
 * "Code": string, code of the weather station, optional
 * "station": string, id of the location of the weather station with the service, required,
 * Coordinates: coordinates weather station, optional
 *  "latitude": float, latitude of the weather station in decimal degrees
 *  "longitude": float, longitude of the weather station in decimal degrees
 */
class WEATHERDATA_EXPORT Locations : public QAbstractListModel
{
    Q_OBJECT

public:
    enum LocationModel {
        PlaceInfo = Qt::UserRole + 1,
        DisplayName,
        Code,
        Station,
        Latitude,
        Longitude,
    };

    explicit Locations(QObject *parent = nullptr);
    ~Locations() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool isError() const;
    void setError();

    void addLocation(const Location &location);
    void addLocations(const QList<Location> &locations);

private:
    bool m_error;

    QList<Location> m_locations;
};

/*!
 * \class Location
 *
 * \brief Data about one location
 */
class WEATHERDATA_EXPORT Location
{
public:
    Location();
    ~Location();

    const std::optional<QString> &placeInfo() const;
    const std::optional<QString> &displayName() const;
    const std::optional<QString> &code() const;
    const std::optional<QString> &station() const;
    const std::optional<QPointF> &coordinates() const;

    void setPlaceInfo(const QString &placeInfo);
    void setDisplayName(const QString &displayName);
    void setCode(const QString &code);
    void setStation(const QString &station);
    void setCoordinates(const QPointF &location);

private:
    std::optional<QString> m_placeInfo;
    std::optional<QString> m_displayName;
    std::optional<QString> m_code;
    std::optional<QString> m_station;
    std::optional<QString> m_state;
    std::optional<QPointF> m_coordinates;
};
