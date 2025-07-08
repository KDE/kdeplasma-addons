/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <weatherdata_export.h>

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QVariant>

#include <qqmlintegration.h>

/*!
 * \class LastObservation
 *
 * \brief Data about last observation
 *
 * "observationTimestamp": datetime (with timezone), time of observation, optional
 * "currentConditions": string, free text string for current weather observation, optional
 * "conditionIcon": string, xdg icon name for current weather observation, optional
 * "temperature": float, using general temperature unit, optional
 * "windchill": float, felt temperature due to wind, using general temperature unit, optional
 * "heatIndex": float, using general temperature unit, optional
 * "humidex": int or string, humidity index (not to be mixed up with heat index), optional
 * "windSpeed": float, average wind speed, optional
 * "windGust": float, max wind gust speed, optional
 * "windDirection": string, wind direction in cardinal directions (up to secondary-intercardinal + VR), optional
 * "visibility": float or string, visibility in distance, optional
 * "pressure": float, air pressure, optional
 * "pressureTendency": PressureTendency enum, "Rising", "Falling", "Steady", optional
 * "UVIndex": int, value in UV index UN standard, optional
 * "UVRating": string, grouping in which UV index is: "Low"0</"Moderate"3</"High"6</"Very high"8</"Extreme"11<, present if UVIndex present
 * so needed?
 * "humidity": float, humidity of the air, optional
 * "dewpoint": float, temperature where water condensates given other conditions, optional
 */
class WEATHERDATA_EXPORT LastObservation
{
    Q_GADGET

    Q_PROPERTY(QVariant observationTimestamp READ getObservationTimestamp CONSTANT)
    Q_PROPERTY(QVariant currentConditions READ getCurrentConditions CONSTANT)
    Q_PROPERTY(QVariant conditionIcon READ getConditionIcon CONSTANT)
    Q_PROPERTY(QVariant temperature READ getTemperature CONSTANT)
    Q_PROPERTY(QVariant windchill READ getWindchill CONSTANT)
    Q_PROPERTY(QVariant heatIndex READ getHeatIndex CONSTANT)
    Q_PROPERTY(QVariant humidex READ getHumidex CONSTANT)
    Q_PROPERTY(QVariant windSpeed READ getWindSpeed CONSTANT)
    Q_PROPERTY(QVariant windGust READ getWindGust CONSTANT)
    Q_PROPERTY(QVariant windDirection READ getWindDirection CONSTANT)
    Q_PROPERTY(QVariant visibility READ getVisibility CONSTANT)
    Q_PROPERTY(QVariant pressure READ getPressure CONSTANT)
    Q_PROPERTY(QVariant pressureTendency READ getPressureTendency CONSTANT)
    Q_PROPERTY(QVariant UVIndex READ getUVIndex CONSTANT)
    Q_PROPERTY(QVariant UVRating READ getUVRating CONSTANT)
    Q_PROPERTY(QVariant humidity READ getHumidity CONSTANT)
    Q_PROPERTY(QVariant dewpoint READ getDewpoint CONSTANT)

public:
    LastObservation();
    ~LastObservation();

    bool isDataPresent() const;

    QVariant getObservationTimestamp() const;
    QVariant getCurrentConditions() const;
    QVariant getConditionIcon() const;
    QVariant getTemperature() const;
    QVariant getWindchill() const;
    QVariant getHeatIndex() const;
    QVariant getHumidex() const;
    QVariant getWindSpeed() const;
    QVariant getWindGust() const;
    QVariant getWindDirection() const;
    QVariant getVisibility() const;
    QVariant getPressure() const;
    QVariant getPressureTendency() const;
    QVariant getUVIndex() const;
    QVariant getUVRating() const;
    QVariant getHumidity() const;
    QVariant getDewpoint() const;

    void setObservationTimestamp(const QDateTime &observationTimestamp);
    void setCurrentConditions(const QString &currentConditions);
    void setConditionIcon(const QString &conditionIcon);
    void setTemperature(qreal temperature);
    void setWindchill(qreal windchill);
    void setHeatIndex(qreal heatIndex);
    void setHumidex(int humidex);
    void setHumidex(const QString &humidexString);
    void setWindSpeed(qreal windSpeed);
    void setWindGust(qreal windGust);
    void setWindDirection(const QString &windDirection);
    void setVisibility(const QString &visibilityString);
    void setVisibility(qreal visibility);
    void setPressure(qreal pressure);
    void setPressureTendency(const QString &pressureTendency);
    void setUVIndex(int UVIndex);
    void setHumidity(qreal humidity);
    void setDewpoint(qreal dewpoint);

private:
    std::optional<QDateTime> m_observationTimestamp;
    std::optional<QString> m_currentConditions;
    std::optional<QString> m_conditionIcon;
    std::optional<qreal> m_temperature;
    std::optional<qreal> m_windchill;
    std::optional<qreal> m_heatIndex;
    std::optional<int> m_humidex;
    std::optional<QString> m_humidexString;
    std::optional<qreal> m_windSpeed;
    std::optional<qreal> m_windGust;
    std::optional<QString> m_windDirection;
    std::optional<qreal> m_visibility;
    std::optional<QString> m_visibilityString;
    std::optional<qreal> m_pressure;
    std::optional<QString> m_pressureTendency;
    std::optional<int> m_UVIndex;
    std::optional<QString> m_UVRating;
    std::optional<qreal> m_humidity;
    std::optional<qreal> m_dewpoint;

    bool m_isDataPresent;
};

Q_DECLARE_METATYPE(LastObservation)
