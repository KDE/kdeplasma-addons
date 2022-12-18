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
    QString persianCalendarSubLabels(const QDate &altDate) const;

    const QCalendar m_calendar;
    const QCalendar::System m_system;
};

QtCalendarProviderPrivate::QtCalendarProviderPrivate(QCalendar::System system)
    : m_calendar(QCalendar(system))
    , m_system(system)
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

    if (m_system == QCalendar::System::Jalali) {
        sublabel.label = persianCalendarSubLabels(altDate);
    } else {
        sublabel.label = QLocale::system().toString(altDate);
    }

    return sublabel;
}

QString QtCalendarProviderPrivate::persianCalendarSubLabels(const QDate &altDate) const
{
    return i18ndc("plasma_calendar_alternatecalendar",
                  "%1 Persian day %2 Persian month %3 Persian year",
                  "%1 %2, %3",
                  QString::number(altDate.day()),
                  m_calendar.standaloneMonthName(QLocale::system(), altDate.month()),
                  QString::number(altDate.year()));
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
