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
 * \class CurrentDay
 *
 * \brief Data about current day
 *
 * "Normal High": float, average highest temperature measured at location, optional
 * "Normal Low": float, average lowest temperature measured at location, optional
 * "Record High Temperature": float, highest temperature ever measured at location, optional
 * "Record Low Temperature": float, lowest temperature ever measured at location, optional
 * "Record Rainfall": float, highest height of rain (precipitation?) ever measured at location, optional
 * "Record Snowfall": float, highest height of snow ever measured at location, optional
 */
class PLASMAWEATHERDATA_EXPORT CurrentDay
{
    Q_GADGET

    Q_PROPERTY(QVariant normalHighTemp READ normalHighTemp CONSTANT)
    Q_PROPERTY(QVariant normalLowTemp READ normalLowTemp CONSTANT)
    Q_PROPERTY(QVariant recordHighTemp READ recordHighTemp CONSTANT)
    Q_PROPERTY(QVariant recordLowTemp READ recordLowTemp CONSTANT)
    Q_PROPERTY(QVariant recordRainfall READ recordRainfall CONSTANT)
    Q_PROPERTY(QVariant recordSnowfall READ recordSnowfall CONSTANT)

public:
    CurrentDay();
    ~CurrentDay();

    bool isDataPresent() const;

    QVariant normalHighTemp() const;
    QVariant normalLowTemp() const;
    QVariant recordHighTemp() const;
    QVariant recordLowTemp() const;
    QVariant recordRainfall() const;
    QVariant recordSnowfall() const;

    void setNormalHighTemp(qreal value);
    void setNormalLowTemp(qreal value);
    void setRecordHighTemp(qreal value);
    void setRecordLowTemp(qreal value);
    void setRecordRainfall(qreal value);
    void setRecordSnowfall(qreal value);

private:
    std::optional<qreal> m_normalHighTemp;
    std::optional<qreal> m_normalLowTemp;
    std::optional<qreal> m_recordHighTemp;
    std::optional<qreal> m_recordLowTemp;
    std::optional<qreal> m_recordRainfall;
    std::optional<qreal> m_recordSnowfall;

    bool m_isDataPresent;
};

Q_DECLARE_METATYPE(CurrentDay)
