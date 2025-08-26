/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "metadata.h"

MetaData::MetaData()
    : m_isDataPresent(false)
{
}

MetaData::~MetaData()
{
}

bool MetaData::isDataPresent() const
{
    return m_isDataPresent;
}

QVariant MetaData::temperatureUnit() const
{
    if (m_temperatureUnit.has_value()) {
        return *m_temperatureUnit;
    }
    return {};
}

QVariant MetaData::windSpeedUnit() const
{
    if (m_windSpeedUnit.has_value()) {
        return *m_windSpeedUnit;
    }
    return {};
}

QVariant MetaData::visibilityUnit() const
{
    if (m_visibilityUnit.has_value()) {
        return *m_visibilityUnit;
    }
    return {};
}

QVariant MetaData::pressureUnit() const
{
    if (m_pressureUnit.has_value()) {
        return *m_pressureUnit;
    }
    return {};
}

QVariant MetaData::humidityUnit() const
{
    if (m_humidityUnit.has_value()) {
        return *m_humidityUnit;
    }
    return {};
}

QVariant MetaData::rainfallUnit() const
{
    if (m_rainfallUnit.has_value()) {
        return *m_rainfallUnit;
    }
    return {};
}

QVariant MetaData::snowfallUnit() const
{
    if (m_snowfallUnit.has_value()) {
        return *m_snowfallUnit;
    }
    return {};
}

QVariant MetaData::precipUnit() const
{
    if (m_precipUnit.has_value()) {
        return *m_precipUnit;
    }
    return {};
}

QString MetaData::credit() const
{
    return m_credit;
}

QVariant MetaData::creditURL() const
{
    if (m_creditURL.has_value()) {
        return *m_creditURL;
    }
    return {};
}

void MetaData::setTemperatureUnit(KUnitConversion::UnitId temperatureUnit)
{
    m_temperatureUnit = temperatureUnit;
    m_isDataPresent = true;
}

void MetaData::setWindSpeedUnit(KUnitConversion::UnitId windSpeedUnit)
{
    m_windSpeedUnit = windSpeedUnit;
    m_isDataPresent = true;
}

void MetaData::setVisibilityUnit(KUnitConversion::UnitId visibilityUnit)
{
    m_visibilityUnit = visibilityUnit;
    m_isDataPresent = true;
}

void MetaData::setPressureUnit(KUnitConversion::UnitId pressureUnit)
{
    m_pressureUnit = pressureUnit;
    m_isDataPresent = true;
}

void MetaData::setHumidityUnit(KUnitConversion::UnitId humidityUnit)
{
    m_humidityUnit = humidityUnit;
    m_isDataPresent = true;
}

void MetaData::setRainfallUnit(KUnitConversion::UnitId rainfallUnit)
{
    m_rainfallUnit = rainfallUnit;
    m_isDataPresent = true;
}

void MetaData::setSnowfallUnit(KUnitConversion::UnitId snowfallUnit)
{
    m_snowfallUnit = snowfallUnit;
    m_isDataPresent = true;
}

void MetaData::setPrecipUnit(KUnitConversion::UnitId precipUnit)
{
    m_precipUnit = precipUnit;
    m_isDataPresent = true;
}

void MetaData::setCredit(const QString &credit)
{
    m_credit = credit;
    m_isDataPresent = true;
}

void MetaData::setCreditURL(const QString &creditURL)
{
    m_creditURL = creditURL;
    m_isDataPresent = true;
}

#include "moc_metadata.cpp"
