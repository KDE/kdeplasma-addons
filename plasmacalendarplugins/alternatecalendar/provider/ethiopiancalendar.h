/*
    SPDX-FileCopyrightText: 2024 Eyobed Awel <eyobedawel@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "abstractcalendarprovider.h"

/**
 * @brief An alternate calendar provider for Ethiopian calendar system.
 */
class EthiopianCalendarProvider : public AbstractCalendarProvider
{
    Q_OBJECT

public:
    explicit EthiopianCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem, const QDate &startDate, const QDate &endDate);
    ~EthiopianCalendarProvider() override;

    QCalendar::YearMonthDay fromGregorian(const QDate &date) const override;
    CalendarEvents::CalendarEventsPlugin::SubLabel subLabel(const QDate &date) const override;

private:
    const std::unique_ptr<class EthiopianCalendarProviderPrivate> d;
};
