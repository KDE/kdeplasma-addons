/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <plasmaweatherdata_export.h>

#include "currentday.h"
#include "futuredays.h"
#include "lastday.h"
#include "lastobservation.h"
#include "metadata.h"
#include "station.h"
#include "warnings.h"

/*!
 * \class Forecast
 *
 * \brief Weather forecast
 *
 * station: forecast station, required,
 * metaData: metadata with additional info, required,
 * lastObservation: last received observation, optional,
 * lastDay: data about last day, optional,
 * currentDay: data about current day, optional
 * warnings: data obout warnings, optional
 * futureDays: data obout future days, optional
 */
class PLASMAWEATHERDATA_EXPORT Forecast
{
    Q_GADGET

    Q_PROPERTY(QVariant station READ station CONSTANT)
    Q_PROPERTY(QVariant metaData READ metaData CONSTANT)
    Q_PROPERTY(QVariant lastObservation READ lastObservation CONSTANT)
    Q_PROPERTY(QVariant lastDay READ lastDay CONSTANT)
    Q_PROPERTY(QVariant currentDay READ currentDay CONSTANT)

    Q_PROPERTY(Warnings *warnings READ warnings CONSTANT)
    Q_PROPERTY(FutureDays *futureDays READ futureDays CONSTANT)

public:
    Forecast();
    ~Forecast();

    bool isError() const;
    QVariant station() const;
    QVariant metaData() const;
    QVariant currentDay() const;
    QVariant lastDay() const;
    QVariant lastObservation() const;
    FutureDays *futureDays() const;
    Warnings *warnings() const;

    void setError();
    void setMetadata(const MetaData &metaData);
    void setStation(const Station &station);
    void setCurrentDay(const CurrentDay &currentDay);
    void setLastDay(const LastDay &lastDay);
    void setLastObservation(const LastObservation &lastObservation);
    void setFutureDays(std::shared_ptr<FutureDays> nextDays);
    void setWarnings(std::shared_ptr<Warnings> warnings);

private:
    bool m_error;

    std::optional<Station> m_station;
    std::optional<MetaData> m_metaData;
    std::optional<CurrentDay> m_currentDay;
    std::optional<LastDay> m_lastDay;
    std::optional<LastObservation> m_lastObservation;
    std::shared_ptr<FutureDays> m_futureDays;
    std::shared_ptr<Warnings> m_warnings;
};

Q_DECLARE_METATYPE(Forecast)
