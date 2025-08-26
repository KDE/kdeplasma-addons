/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "currentday.h"

CurrentDay::CurrentDay()
    : m_isDataPresent(false)
{
}

CurrentDay::~CurrentDay()
{
}

bool CurrentDay::isDataPresent() const
{
    return m_isDataPresent;
}

QVariant CurrentDay::normalHighTemp() const
{
    if (m_recordLowTemp.has_value()) {
        return *m_normalHighTemp;
    }
    return {};
}
QVariant CurrentDay::normalLowTemp() const
{
    if (m_normalLowTemp.has_value()) {
        return *m_normalLowTemp;
    }
    return {};
}

QVariant CurrentDay::recordHighTemp() const
{
    if (m_recordRainfall.has_value()) {
        return *m_recordHighTemp;
    }
    return {};
}

QVariant CurrentDay::recordLowTemp() const
{
    if (m_recordLowTemp.has_value()) {
        return *m_recordLowTemp;
    }
    return {};
}

QVariant CurrentDay::recordRainfall() const
{
    if (m_recordRainfall.has_value()) {
        return *m_recordRainfall;
    }
    return {};
}

QVariant CurrentDay::recordSnowfall() const
{
    if (m_recordSnowfall.has_value()) {
        return *m_recordSnowfall;
    }
    return {};
}

void CurrentDay::setNormalHighTemp(qreal value)
{
    m_normalHighTemp = value;
    m_isDataPresent = true;
}

void CurrentDay::setNormalLowTemp(qreal value)
{
    m_normalLowTemp = value;
    m_isDataPresent = true;
}

void CurrentDay::setRecordHighTemp(qreal value)
{
    m_recordHighTemp = value;
    m_isDataPresent = true;
}

void CurrentDay::setRecordLowTemp(qreal value)
{
    m_recordLowTemp = value;
    m_isDataPresent = true;
}

void CurrentDay::setRecordRainfall(qreal value)
{
    m_recordRainfall = value;
    m_isDataPresent = true;
}

void CurrentDay::setRecordSnowfall(qreal value)
{
    m_recordSnowfall = value;
    m_isDataPresent = true;
}

#include "moc_currentday.cpp"
