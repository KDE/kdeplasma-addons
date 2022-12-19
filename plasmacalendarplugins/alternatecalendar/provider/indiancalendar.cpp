/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "indiancalendar.h"

class IndianCalendarProviderPrivate : public ICUCalendarPrivate
{
public:
    explicit IndianCalendarProviderPrivate();

    QDate fromGregorian(const QDate &_date);
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

QDate IndianCalendarProviderPrivate::fromGregorian(const QDate &_date)
{
    if (U_FAILURE(m_errorCode) || !_date.isValid() || !setDate(_date)) {
        return QDate();
    }

    return date();
}

CalendarEvents::CalendarEventsPlugin::SubLabel IndianCalendarProviderPrivate::subLabels(const QDate &date)
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};

    if (U_FAILURE(m_errorCode) || !date.isValid() || !setDate(date)) {
        return sublabel;
    }

    static const std::array<QString, 12> monthNames{
        i18ndc("plasma_calendar_alternatecalendar", "Month name in Indian National Calendar", "Chaitra"),
        i18ndc("plasma_calendar_alternatecalendar", "Month name in Indian National Calendar", "Vaisākha"),
        i18ndc("plasma_calendar_alternatecalendar", "Month name in Indian National Calendar", "Jyēshtha"),
        i18ndc("plasma_calendar_alternatecalendar", "Month name in Indian National Calendar", "Āshādha"),
        i18ndc("plasma_calendar_alternatecalendar", "Month name in Indian National Calendar", "Shrāvana"),
        i18ndc("plasma_calendar_alternatecalendar", "Month name in Indian National Calendar", "Bhādra"),
        i18ndc("plasma_calendar_alternatecalendar", "Month name in Indian National Calendar", "Āshwin"),
        i18ndc("plasma_calendar_alternatecalendar", "Month name in Indian National Calendar", "Kārtika"),
        i18ndc("plasma_calendar_alternatecalendar", "Month name in Indian National Calendar", "Mārgaśīrṣa"),
        i18ndc("plasma_calendar_alternatecalendar", "Month name in Indian National Calendar", "Pausha"),
        i18ndc("plasma_calendar_alternatecalendar", "Month name in Indian National Calendar", "Māgha"),
        i18ndc("plasma_calendar_alternatecalendar", "Month name in Indian National Calendar", "Phālguna"),
    };

    sublabel.dayLabel = QString::number(day());
    sublabel.label = i18ndc("plasma_calendar_alternatecalendar",
                            "@label %1 day %2 month name in India National Calendar %3 year",
                            "%1 %2, %3",
                            sublabel.dayLabel,
                            monthNames[month() - 1],
                            QString::number(year()));
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

QDate IndianCalendarProvider::fromGregorian(const QDate &date) const
{
    return d->fromGregorian(date);
}

CalendarEvents::CalendarEventsPlugin::SubLabel IndianCalendarProvider::subLabels(const QDate &date) const
{
    return d->subLabels(date);
}
