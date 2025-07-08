/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <weatherdata_export.h>

#include <QObject>
#include <QString>
#include <QVariant>

#include <qqmlintegration.h>

/*!
 * \class Station
 *
 * \brief Data about the weather station
 *
 * Data about the weather station:
 * "station": string, id of the location of the weather station with the service, required, TODO: ensure it's id
 * "place": string, display name of the location of the weather station, required, TODO: what details here, country?
 * "country": string, display name of country of the weather station, optional
 * Coordinates: coordinates weather station, optional
 *  "latitude": float, latitude of the weather station in decimal degrees
 *  "longitude": float, longitude of the weather station in decimal degrees
 */
class WEATHERDATA_EXPORT Station
{
    Q_GADGET

    Q_PROPERTY(QVariant station READ getStation CONSTANT)
    Q_PROPERTY(QVariant place READ getPlace CONSTANT)
    Q_PROPERTY(QVariant country READ getCountry CONSTANT)
    Q_PROPERTY(QVariant latitude READ getLatitude CONSTANT)
    Q_PROPERTY(QVariant longitude READ getLongitude CONSTANT)

public:
    Station();
    ~Station();

    bool isDataPresent() const;

    QVariant getStation() const;
    QVariant getPlace() const;
    QVariant getRegion() const;
    QVariant getCountry() const;
    QVariant getLatitude() const;
    QVariant getLongitude() const;

    void setStation(const QString &station);
    void setPlace(const QString &place);
    void setRegion(const QString &region);
    void setCountry(const QString &country);
    void setCoordinates(qreal latitude, qreal longitude);

private:
    std::optional<QString> m_station;
    std::optional<QString> m_place;
    std::optional<QString> m_region;
    std::optional<QString> m_country;
    std::optional<qreal> m_latitude;
    std::optional<qreal> m_longitude;

    bool m_isDataPresent;
};

Q_DECLARE_METATYPE(Station)
