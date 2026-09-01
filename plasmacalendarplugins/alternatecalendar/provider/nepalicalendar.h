/*
    SPDX-FileCopyrightText: 2026 Satya Prakash Dahal <dahal.satya321@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "abstractcalendarprovider.h"

/**
 * @brief An alternate calendar provider for the Bikram Sambat (Nepali) calendar system.
 *
 * The official Bikram Sambat is set by the government of Nepal and published via yearly
 * patro. Its month lengths only exist as a lookup table (BS 2000-2099, approximately
 * Gregorian 1943-04-14 to 2042-04-13). Dates outside that range are approximated using
 * the mean month length derived from the table and are flagged as approximate in the
 * sub-label.
 */
class NepaliCalendarProvider : public AbstractCalendarProvider
{
    Q_OBJECT

public:
    explicit NepaliCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem, const QDate &startDate, const QDate &endDate);
    ~NepaliCalendarProvider() override;

    QCalendar::YearMonthDay fromGregorian(const QDate &date) const override;
    CalendarEvents::CalendarEventsPlugin::SubLabel subLabel(const QDate &date) const override;
};
