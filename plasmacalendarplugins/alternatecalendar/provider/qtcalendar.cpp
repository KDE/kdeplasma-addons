/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qtcalendar.h"

QtCalendarProvider::QtCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem)
    : AbstractCalendarProvider(parent, calendarSystem)
    , m_calendar(QCalendar(static_cast<QCalendar::System>(calendarSystem)))
{
    Q_ASSERT(m_calendarSystem <= static_cast<CalendarSystem::System>(QCalendar::System::Last));
}

QtCalendarProvider::~QtCalendarProvider()
{
}

QCalendar::YearMonthDay QtCalendarProvider::fromGregorian(const QDate &date) const
{
    if (!date.isValid()) {
        return {};
    }

    return m_calendar.partsFromDate(date);
}

CalendarEvents::CalendarEventsPlugin::SubLabel QtCalendarProvider::subLabels(const QDate &date) const
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};

    if (!date.isValid()) {
        return sublabel;
    }

    const QCalendar::YearMonthDay altDate = fromGregorian(date);
    sublabel.label = i18ndc("plasma_calendar_alternatecalendar",
                            "@label %1 day %2 month name %3 year",
                            "%1 %2, %3",
                            QString::number(altDate.day),
                            m_calendar.standaloneMonthName(QLocale::system(), altDate.month, altDate.year),
                            QString::number(altDate.year));

    return sublabel;
}
