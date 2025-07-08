/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "lastday.h"

LastDay::LastDay()
    : m_isDataPresent(false)
{
}

LastDay::~LastDay()
{
}

bool LastDay::isDataPresent() const
{
    return m_isDataPresent;
}

QVariant LastDay::getNormalHighTemp() const
{
    if (m_normalHighTemp.has_value()) {
        return *m_normalHighTemp;
    }
    return {};
}
QVariant LastDay::getNormalLowTemp() const
{
    if (m_normalLowTemp.has_value()) {
        return *m_normalLowTemp;
    }
    return {};
}
QVariant LastDay::getPrecipTotal() const
{
    if (m_precipTotal.has_value()) {
        return *m_precipTotal;
    }
    return {};
}

void LastDay::setNormalHighTemp(qreal normalHighTemp)
{
    m_normalHighTemp = normalHighTemp;
    m_isDataPresent = true;
}

void LastDay::setNormalLowTemp(qreal normalLowTemp)
{
    m_normalLowTemp = normalLowTemp;
    m_isDataPresent = true;
}

void LastDay::setPrecipTotal(qreal precipTotal)
{
    m_precipTotal = precipTotal;
    m_isDataPresent = true;
}

#include "moc_lastday.cpp"
