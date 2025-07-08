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
class WEATHERDATA_EXPORT CurrentDay
{
    Q_GADGET

    Q_PROPERTY(QVariant normalHighTemp READ getNormalHighTemp CONSTANT)
    Q_PROPERTY(QVariant normalLowTemp READ getNormalLowTemp CONSTANT)
    Q_PROPERTY(QVariant recordHighTemp READ getRecordHighTemp CONSTANT)
    Q_PROPERTY(QVariant recordLowTemp READ getRecordLowTemp CONSTANT)
    Q_PROPERTY(QVariant recordRainfall READ getRecordRainfall CONSTANT)
    Q_PROPERTY(QVariant recordSnowfall READ getRecordSnowfall CONSTANT)

public:
    CurrentDay();
    ~CurrentDay();

    bool isDataPresent() const;

    QVariant getNormalHighTemp() const;
    QVariant getNormalLowTemp() const;
    QVariant getRecordHighTemp() const;
    QVariant getRecordLowTemp() const;
    QVariant getRecordRainfall() const;
    QVariant getRecordSnowfall() const;

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
