/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qtcalendar.h"

class QtCalendarProviderPrivate
{
public:
    explicit QtCalendarProviderPrivate(QCalendar::System system);
    ~QtCalendarProviderPrivate();

    QCalendar::YearMonthDay fromGregorian(const QDate &date) const;
    CalendarEvents::CalendarEventsPlugin::SubLabel subLabels(const QDate &date) const;

private:
    const QCalendar m_calendar;
};

QtCalendarProviderPrivate::QtCalendarProviderPrivate(QCalendar::System system)
    : m_calendar(QCalendar(system))
{
}

QtCalendarProviderPrivate::~QtCalendarProviderPrivate()
{
}

QCalendar::YearMonthDay QtCalendarProviderPrivate::fromGregorian(const QDate &date) const
{
    if (!date.isValid()) {
        return {};
    }

    return m_calendar.partsFromDate(date);
}

CalendarEvents::CalendarEventsPlugin::SubLabel QtCalendarProviderPrivate::subLabels(const QDate &date) const
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

QtCalendarProvider::QtCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem)
    : AbstractCalendarProvider(parent, calendarSystem)
    , d(new QtCalendarProviderPrivate(static_cast<QCalendar::System>(calendarSystem)))
{
    Q_ASSERT(m_calendarSystem <= static_cast<CalendarSystem::System>(QCalendar::System::Last));
}

QtCalendarProvider::~QtCalendarProvider()
{
}

QCalendar::YearMonthDay QtCalendarProvider::fromGregorian(const QDate &date) const
{
    return d->fromGregorian(date);
}

CalendarEvents::CalendarEventsPlugin::SubLabel QtCalendarProvider::subLabels(const QDate &date) const
{
    return d->subLabels(date);
}
