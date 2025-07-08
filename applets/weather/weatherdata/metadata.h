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

#include <KUnitConversion/Converter>

#include <qqmlintegration.h>

/*!
 * \class Warnings
 *
 * \brief Data about the data:
 *
 * "Temperature Unit": int, kunitconversion enum number for the unit with all temperature values, required if temperatures are given
 * "Wind Speed Unit": int, kunitconversion enum number for the unit with all wind speed values, required if wind speeds are given
 * "Visibility Unit": int, kunitconversion enum number for the unit with all visibility values, required if visibilities are given
 * "Pressure Unit": int, kunitconversion enum number for the unit with all pressure values, required if pressures are given
 * "Humidity Unit": int, kunitconversion enum number for the unit with all humidity values, required if humidities are given TODO: any other unit than percent
 * "Rainfall Unit": int, kunitconversion enum number for the unit with all rainfall values, required if rainfalls are given
 * "Snowfall Unit": int, kunitconversion enum number for the unit with Record Snowfall, required if snowfall is given
 * "Precip Unit": int, kunitconversion enum number for the unit with recip, required if precipitation is given
 * "Credit": string, credit line for the data, required
 * "Credit Url": string, url related to the credit for the data (can be also webpage with more forecast), optional
 */
class WEATHERDATA_EXPORT MetaData
{
    Q_GADGET

    Q_PROPERTY(QString credit READ getCredit CONSTANT)
    Q_PROPERTY(QVariant creditURL READ getCreditURL CONSTANT)
    Q_PROPERTY(QVariant temperatureUnit READ getTemperatureUnit CONSTANT)
    Q_PROPERTY(QVariant windSpeedUnit READ getWindSpeedUnit CONSTANT)
    Q_PROPERTY(QVariant visibilityUnit READ getVisibilityUnit CONSTANT)
    Q_PROPERTY(QVariant pressureUnit READ getPressureUnit CONSTANT)
    Q_PROPERTY(QVariant humidityUnit READ getHumidityUnit CONSTANT)
    Q_PROPERTY(QVariant rainfallUnit READ getRainfallUnit CONSTANT)
    Q_PROPERTY(QVariant snowfallUnit READ getSnowfallUnit CONSTANT)
    Q_PROPERTY(QVariant precipUnit READ getPrecipUnit CONSTANT)

public:
    MetaData();
    ~MetaData();

    bool isDataPresent() const;

    QString getCredit() const;
    QVariant getCreditURL() const;
    QVariant getTemperatureUnit() const;
    QVariant getWindSpeedUnit() const;
    QVariant getVisibilityUnit() const;
    QVariant getPressureUnit() const;
    QVariant getHumidityUnit() const;
    QVariant getRainfallUnit() const;
    QVariant getSnowfallUnit() const;
    QVariant getPrecipUnit() const;

    void setCredit(const QString &credit);
    void setCreditURL(const QString &creditURL);
    void setTemperatureUnit(KUnitConversion::UnitId temperatureUnit);
    void setWindSpeedUnit(KUnitConversion::UnitId windSpeedUnit);
    void setVisibilityUnit(KUnitConversion::UnitId visibilityUnit);
    void setPressureUnit(KUnitConversion::UnitId pressureUnit);
    void setHumidityUnit(KUnitConversion::UnitId humidityUnit);
    void setRainfallUnit(KUnitConversion::UnitId rainfallUnit);
    void setSnowfallUnit(KUnitConversion::UnitId snowfallUnit);
    void setPrecipUnit(KUnitConversion::UnitId precipUnit);

private:
    QString m_credit;
    std::optional<QString> m_creditURL;
    std::optional<int> m_temperatureUnit;
    std::optional<int> m_windSpeedUnit;
    std::optional<int> m_visibilityUnit;
    std::optional<int> m_pressureUnit;
    std::optional<int> m_humidityUnit;
    std::optional<int> m_rainfallUnit;
    std::optional<int> m_snowfallUnit;
    std::optional<int> m_precipUnit;

    bool m_isDataPresent;
};

Q_DECLARE_METATYPE(MetaData)
