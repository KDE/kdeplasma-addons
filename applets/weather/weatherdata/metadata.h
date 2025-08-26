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

    Q_PROPERTY(QString credit READ credit CONSTANT)
    Q_PROPERTY(QVariant creditURL READ creditURL CONSTANT)
    Q_PROPERTY(QVariant temperatureUnit READ temperatureUnit CONSTANT)
    Q_PROPERTY(QVariant windSpeedUnit READ windSpeedUnit CONSTANT)
    Q_PROPERTY(QVariant visibilityUnit READ visibilityUnit CONSTANT)
    Q_PROPERTY(QVariant pressureUnit READ pressureUnit CONSTANT)
    Q_PROPERTY(QVariant humidityUnit READ humidityUnit CONSTANT)
    Q_PROPERTY(QVariant rainfallUnit READ rainfallUnit CONSTANT)
    Q_PROPERTY(QVariant snowfallUnit READ snowfallUnit CONSTANT)
    Q_PROPERTY(QVariant precipUnit READ precipUnit CONSTANT)

public:
    MetaData();
    ~MetaData();

    bool isDataPresent() const;

    QString credit() const;
    QVariant creditURL() const;
    QVariant temperatureUnit() const;
    QVariant windSpeedUnit() const;
    QVariant visibilityUnit() const;
    QVariant pressureUnit() const;
    QVariant humidityUnit() const;
    QVariant rainfallUnit() const;
    QVariant snowfallUnit() const;
    QVariant precipUnit() const;

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
