/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "../alternatecalendarplugin.h"

/**
 * @short An abstract base class for alternate calendar providers.
 *
 * This class serves as abstract base class for alternate calendar
 * provider implementations.
 */
class AbstractCalendarProvider : public QObject
{
    Q_OBJECT

public:
    explicit AbstractCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem);
    ~AbstractCalendarProvider() override;

    /**
     * Converts Gregorian date to alternate calendar date
     *
     * @param date Gregorian date
     * @return Alternate calendar date
     */
    virtual QCalendar::YearMonthDay fromGregorian(const QDate &date) const;

    /**
     * Gets the sub-labels for the specific Gregorian date
     *
     * @param date Gregorian date
     * @return Sub-labels for the specific Gregorian date
     */
    virtual CalendarEvents::CalendarEventsPlugin::SubLabel subLabels(const QDate &date) const;

protected:
    const CalendarSystem::System m_calendarSystem;
};
