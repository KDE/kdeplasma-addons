/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "abstractcalendarprovider.h"

AbstractCalendarProvider::AbstractCalendarProvider(QObject *parent,
                                                   CalendarSystem::System calendarSystem,
                                                   std::vector<QDate> &&alternateDates,
                                                   std::vector<QDate> &&sublabelDates,
                                                   int dateOffset)
    : QObject(parent)
    , m_calendarSystem(calendarSystem)
    , m_alternateDates(std::move(alternateDates))
    , m_sublabelDates(std::move(sublabelDates))
    , m_dateOffset(dateOffset)
{
}

AbstractCalendarProvider::~AbstractCalendarProvider()
{
}

QCalendar::YearMonthDay AbstractCalendarProvider::fromGregorian(const QDate &date) const
{
    if (!date.isValid()) {
        return {};
    }

    return QCalendar::YearMonthDay(date.year(), date.month(), date.day());
}

CalendarEvents::CalendarEventsPlugin::SubLabel AbstractCalendarProvider::subLabel([[maybe_unused]] const QDate &date) const
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};
    sublabel.priority = CalendarEvents::CalendarEventsPlugin::SubLabelPriority::Low;

    return sublabel;
}

void AbstractCalendarProvider::run()
{
    QHash<QDate, QCalendar::YearMonthDay> alternateDatesData;
    for (const QDate &date : m_alternateDates) {
        const QDate offsetDate = date.addDays(m_dateOffset);
        const QCalendar::YearMonthDay alt = fromGregorian(offsetDate);
        // Always insert all alternate dates to make cache check work
        alternateDatesData.insert(date, QCalendar::YearMonthDay(alt.year, alt.month, alt.day));
    }

    QHash<QDate, CalendarEvents::CalendarEventsPlugin::SubLabel> sublabelData;
    for (const QDate &date : m_sublabelDates) {
        const QDate offsetDate = date.addDays(m_dateOffset);
        sublabelData.insert(date, subLabel(offsetDate));
    }

    Q_EMIT dataReady(alternateDatesData, sublabelData);
}

#include "moc_abstractcalendarprovider.cpp"