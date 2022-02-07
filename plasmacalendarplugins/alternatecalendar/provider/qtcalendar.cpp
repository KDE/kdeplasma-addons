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

    QDate fromGregorian(const QDate &date) const;
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

QDate QtCalendarProviderPrivate::fromGregorian(const QDate &date) const
{
    if (!date.isValid()) {
        return QDate();
    }

    const QCalendar::YearMonthDay ymd = m_calendar.partsFromDate(date);
    return QDate(ymd.year, ymd.month, ymd.day);
}

CalendarEvents::CalendarEventsPlugin::SubLabel QtCalendarProviderPrivate::subLabels(const QDate &date) const
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};

    if (!date.isValid()) {
        return sublabel;
    }

    const QDate altDate = fromGregorian(date);
    sublabel.label = QLocale::system().toString(altDate);

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

QDate QtCalendarProvider::fromGregorian(const QDate &date) const
{
    return d->fromGregorian(date);
}

CalendarEvents::CalendarEventsPlugin::SubLabel QtCalendarProvider::subLabels(const QDate &date) const
{
    return d->subLabels(date);
}
