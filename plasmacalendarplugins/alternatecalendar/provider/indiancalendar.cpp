/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "indiancalendar.h"

class IndianCalendarProviderPrivate : public ICUCalendarPrivate
{
public:
    explicit IndianCalendarProviderPrivate();

    QCalendar::YearMonthDay fromGregorian(const QDate &_date);
    CalendarEvents::CalendarEventsPlugin::SubLabel subLabels(const QDate &date);
};

IndianCalendarProviderPrivate::IndianCalendarProviderPrivate()
    : ICUCalendarPrivate()
{
    if (U_FAILURE(m_errorCode)) {
        return; // Failed to create m_GregorianCalendar
    }

    m_calendar.reset(icu::Calendar::createInstance("en_US@calendar=indian", m_errorCode));
}

QCalendar::YearMonthDay IndianCalendarProviderPrivate::fromGregorian(const QDate &_date)
{
    if (U_FAILURE(m_errorCode) || !_date.isValid() || !setDate(_date)) {
        return {};
    }

    return date();
}

CalendarEvents::CalendarEventsPlugin::SubLabel IndianCalendarProviderPrivate::subLabels(const QDate &date)
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};

    if (U_FAILURE(m_errorCode) || !date.isValid() || !setDate(date)) {
        return sublabel;
    }

    sublabel.dayLabel = QString::number(day());
    sublabel.label = QLocale(QLocale::English, QLocale::India).toString(QDate(year(), month(), day()));
    sublabel.priority = CalendarEvents::CalendarEventsPlugin::SubLabelPriority::Low;

    return sublabel;
}

IndianCalendarProvider::IndianCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem)
    : AbstractCalendarProvider(parent, calendarSystem)
    , d(std::make_unique<IndianCalendarProviderPrivate>())
{
    Q_ASSERT(m_calendarSystem == CalendarSystem::Indian);
}

IndianCalendarProvider::~IndianCalendarProvider()
{
}

QCalendar::YearMonthDay IndianCalendarProvider::fromGregorian(const QDate &date) const
{
    return d->fromGregorian(date);
}

CalendarEvents::CalendarEventsPlugin::SubLabel IndianCalendarProvider::subLabels(const QDate &date) const
{
    return d->subLabels(date);
}
