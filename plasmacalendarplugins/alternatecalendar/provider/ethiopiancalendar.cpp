/*
    SPDX-FileCopyrightText: 2024 Eyobed Awel <eyobedawel@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ethiopiancalendar.h"

#include "icucalendar_p.h"

class EthiopianCalendarProviderPrivate : public ICUCalendarPrivate
{
public:
    explicit EthiopianCalendarProviderPrivate();

    QString formattedDateString(const icu::UnicodeString &str) const;
    QString formattedDateStringInNativeLanguage(const icu::UnicodeString &str) const;

    QCalendar::YearMonthDay fromGregorian(const QDate &_date);
    CalendarEvents::CalendarEventsPlugin::SubLabel subLabel(const QDate &date);

private:
    icu::Locale m_ethiopianLocale;
    icu::Locale m_nativeLocale;
};

EthiopianCalendarProviderPrivate::EthiopianCalendarProviderPrivate()
    : ICUCalendarPrivate()
    , m_ethiopianLocale(icu::Locale("am_ET", nullptr, nullptr, "calendar=ethiopic"))
    , m_nativeLocale(icu::Locale(QLocale::system().name().toLatin1().constData(), nullptr, nullptr, "calendar=ethiopic"))
{
    if (U_FAILURE(m_errorCode)) {
        return;
    }

    m_calendar.reset(icu::Calendar::createInstance(icu::Locale("am_ET@calendar=ethiopic"), m_errorCode));
}

QString EthiopianCalendarProviderPrivate::formattedDateString(const icu::UnicodeString &str) const
{
    UErrorCode errorCode = U_ZERO_ERROR;
    icu::UnicodeString dateString;
    icu::SimpleDateFormat formatter(str, m_ethiopianLocale, errorCode);
    formatter.setCalendar(*m_calendar);
    formatter.format(m_calendar->getTime(errorCode), dateString);

    return QStringView(dateString.getBuffer(), dateString.length()).toString();
}

QString EthiopianCalendarProviderPrivate::formattedDateStringInNativeLanguage(const icu::UnicodeString &str) const
{
    UErrorCode errorCode = U_ZERO_ERROR;
    icu::UnicodeString dateString;
    icu::SimpleDateFormat formatter(str, m_nativeLocale, errorCode);
    formatter.setCalendar(*m_calendar);
    formatter.format(m_calendar->getTime(errorCode), dateString);

    return QStringView(dateString.getBuffer(), dateString.length()).toString();
}

QCalendar::YearMonthDay EthiopianCalendarProviderPrivate::fromGregorian(const QDate &_date)
{
    if (U_FAILURE(m_errorCode) || !_date.isValid() || !setDate(_date)) {
        return {};
    }

    return date();
}

CalendarEvents::CalendarEventsPlugin::SubLabel EthiopianCalendarProviderPrivate::subLabel(const QDate &date)
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};

    if (U_FAILURE(m_errorCode) || !date.isValid() || !setDate(date)) {
        return sublabel;
    }

    const bool isLocaleAmharic = QLocale::system().language() == QLocale::Amharic;

    sublabel.dayLabel = isLocaleAmharic ? formattedDateString("d") : QString::number(day());
    const QString ethiopianDateString = formattedDateString("MMMM d, y"); 

    sublabel.label = isLocaleAmharic ? ethiopianDateString
                                     : i18ndc("plasma_calendar_alternatecalendar",
                                              "%1 Day number %2 Translated month name in Ethiopian calendar %3 Year number %4 Full date in Ethiopian",
                                              "%1 %2, %3 (%4)",
                                              QString::number(day()),
                                              formattedDateStringInNativeLanguage("MMMM"),
                                              QString::number(year()),
                                              ethiopianDateString);
    sublabel.priority = CalendarEvents::CalendarEventsPlugin::SubLabelPriority::Low;

    return sublabel;
}

EthiopianCalendarProvider::EthiopianCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem, const QDate &startDate, const QDate &endDate)
    : AbstractCalendarProvider(parent, calendarSystem, startDate, endDate)
    , d(std::make_unique<EthiopianCalendarProviderPrivate>())
{
    Q_ASSERT(m_calendarSystem == CalendarSystem::Ethiopian);
}

EthiopianCalendarProvider::~EthiopianCalendarProvider()
{
}

QCalendar::YearMonthDay EthiopianCalendarProvider::fromGregorian(const QDate &date) const
{
    return d->fromGregorian(date);
}

CalendarEvents::CalendarEventsPlugin::SubLabel EthiopianCalendarProvider::subLabel(const QDate &date) const
{
    return d->subLabel(date);
}
