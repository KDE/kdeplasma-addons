/*
    SPDX-FileCopyrightText: 2026 Trần Nam Tuấn <tuantran1632001@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "abstractcalendarprovider.h"

/*!
    @brief An alternate calendar provider for Vietnamese calendar system.

    This class presents an alternate calendar provider for Vietnamese calendar system.
    Structurely, it is the same as Chinese calendar. However, due to different timezones,
    roughly every 23 years, the twos generally drifted apart by one day. There are also some
    cultural differences, e.g. different Zodiac names.
 */
class VietnameseCalendarProvider : public AbstractCalendarProvider
{
    Q_OBJECT
public:
    VietnameseCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem, const QDate &startDate, const QDate &endDate);
    ~VietnameseCalendarProvider() override;

    QCalendar::YearMonthDay fromGregorian(const QDate &date) const override;

    CalendarEvents::CalendarEventsPlugin::SubLabel subLabel(const QDate &date) const override;

private:
    const std::unique_ptr<class VietnameseCalendarProviderPrivate> d;
};
