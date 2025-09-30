/*
 * SPDX-FileCopyrightText: 2025 Bohdan Onofriichuk <bogdan.onofriuchuk@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <plasmaweatherdata_export.h>

#include <QObject>
#include <QString>
#include <QVariant>

#include <qqmlintegration.h>

/*!
 * \class LastDay
 *
 * \brief Data about last day
 *
 * "normalHighTemp": float, highest temperature at location, optional
 * "normalLowTemp": float, lowest temperature at location, optional
 * "precipTotal": float, total precipitation over day, optional TODO: "Trace" injected, should be dealt with at display side?
 */
class PLASMAWEATHERDATA_EXPORT LastDay
{
    Q_GADGET

    Q_PROPERTY(QVariant normalHighTemp READ normalHighTemp CONSTANT)
    Q_PROPERTY(QVariant normalLowTemp READ normalLowTemp CONSTANT)
    Q_PROPERTY(QVariant precipTotal READ precipTotal CONSTANT)

public:
    LastDay();
    ~LastDay();

    bool isDataPresent() const;

    QVariant normalHighTemp() const;
    QVariant normalLowTemp() const;
    QVariant precipTotal() const;

    void setNormalHighTemp(qreal normalHighTemp);
    void setNormalLowTemp(qreal normalLowTemp);
    void setPrecipTotal(qreal precipTotal);

private:
    std::optional<qreal> m_normalHighTemp;
    std::optional<qreal> m_normalLowTemp;
    std::optional<qreal> m_precipTotal;

    bool m_isDataPresent;
};

Q_DECLARE_METATYPE(LastDay)
