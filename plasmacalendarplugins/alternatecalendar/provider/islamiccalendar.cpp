/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "islamiccalendar.h"

#include "icucalendar_p.h"

class IslamicCalendarProviderPrivate : public ICUCalendarPrivate
{
public:
    explicit IslamicCalendarProviderPrivate(CalendarSystem::System calendarSystem);

    /**
     * For formatting, see the documentation of SimpleDateFormat:
     * https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1SimpleDateFormat.html#details
     */
    QString formattedDateString(const icu::UnicodeString &str) const;
    QString formattedDateStringInNativeLanguage(const icu::UnicodeString &str) const;

    QCalendar::YearMonthDay fromGregorian(const QDate &_date);
    CalendarEvents::CalendarEventsPlugin::SubLabel subLabel(const QDate &date);

private:
    // See https://unicode-org.github.io/icu/userguide/locale/#keywords for available keywords
    icu::Locale m_arabicLocale;
    icu::Locale m_nativeLocale;
};

IslamicCalendarProviderPrivate::IslamicCalendarProviderPrivate(CalendarSystem::System calendarSystem)
    : ICUCalendarPrivate()
{
    if (U_FAILURE(m_errorCode)) {
        return; // Failed to create m_GregorianCalendar
    }

    // See https://github.com/unicode-org/cldr/blob/main/common/bcp47/number.xml for available number systems
    // See https://cldr.unicode.org/development/development-process/design-proposals/islamic-calendar-types for available Islamic caneldar types
    switch (calendarSystem) {
    case CalendarSystem::Jalali: {
        m_arabicLocale = icu::Locale("ar_IR", 0, 0, "calendar=persian;numbers=arab");
        m_nativeLocale = icu::Locale(QLocale::system().name().toLatin1(), 0, 0, "calendar=persian;");
        break;
    }
    case CalendarSystem::Islamic: {
        m_arabicLocale = icu::Locale("ar_SA", 0, 0, "calendar=islamic;numbers=arab");
        m_nativeLocale = icu::Locale(QLocale::system().name().toLatin1(), 0, 0, "calendar=islamic;");
        break;
    }
    case CalendarSystem::IslamicCivil: {
        m_arabicLocale = icu::Locale("ar_SA", 0, 0, "calendar=islamic-civil;numbers=arab");
        m_nativeLocale = icu::Locale(QLocale::system().name().toLatin1(), 0, 0, "calendar=islamic-civil;");
        break;
    }
    case CalendarSystem::IslamicUmalqura: {
        m_arabicLocale = icu::Locale("ar_SA", 0, 0, "calendar=islamic-umalqura;numbers=arab");
        m_nativeLocale = icu::Locale(QLocale::system().name().toLatin1(), 0, 0, "calendar=islamic-umalqura;");
        break;
    }
    default:
        Q_UNREACHABLE();
    }

    m_calendar.reset(icu::Calendar::createInstance(m_arabicLocale, m_errorCode));
}

QString IslamicCalendarProviderPrivate::formattedDateString(const icu::UnicodeString &str) const
{
    UErrorCode errorCode = U_ZERO_ERROR;
    icu::UnicodeString dateString;
    icu::SimpleDateFormat formatter(str, m_arabicLocale, errorCode);
    formatter.setCalendar(*m_calendar);
    formatter.format(m_calendar->getTime(errorCode), dateString);

    std::string utf8Str;
    dateString.toUTF8String<std::string>(utf8Str);

    return QString::fromStdString(utf8Str);
}

QString IslamicCalendarProviderPrivate::formattedDateStringInNativeLanguage(const icu::UnicodeString &str) const
{
    UErrorCode errorCode = U_ZERO_ERROR;
    icu::UnicodeString dateString;
    icu::SimpleDateFormat formatter(str, m_nativeLocale, errorCode);
    formatter.setCalendar(*m_calendar);
    formatter.format(m_calendar->getTime(errorCode), dateString);

    std::string utf8Str;
    dateString.toUTF8String<std::string>(utf8Str);

    return QString::fromStdString(utf8Str);
}

QCalendar::YearMonthDay IslamicCalendarProviderPrivate::fromGregorian(const QDate &_date)
{
    if (U_FAILURE(m_errorCode) || !_date.isValid() || !setDate(_date)) {
        return {};
    }

    return date();
}

CalendarEvents::CalendarEventsPlugin::SubLabel IslamicCalendarProviderPrivate::subLabel(const QDate &date)
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};

    if (U_FAILURE(m_errorCode) || !date.isValid() || !setDate(date)) {
        return sublabel;
    }

    const bool isLocaleArabicOrPersian = QLocale::system().language() == QLocale::Arabic || QLocale::system().language() == QLocale::Persian;

    sublabel.dayLabel = isLocaleArabicOrPersian ? formattedDateString("d") : QString::number(day());
    // From QLocale(QLocale::Arabic).dateFormat() and QLocale(QLocale::Persian).dateFormat()
    const QString arabicDateString = formattedDateString("d MMMM yyyy");
    // Translated month names are available in https://github.com/unicode-org/icu/tree/main/icu4c/source/data/locales
    sublabel.label = isLocaleArabicOrPersian
        ? arabicDateString
        : i18ndc("plasma_calendar_alternatecalendar",
                 "@label %1 Day number %2 Month name in Islamic Calendar %3 Year number %4 Islamic calendar date in Arabic",
                 "%1 %2, %3 (%4)",
                 QString::number(day()),
                 formattedDateStringInNativeLanguage("MMMM"),
                 QString::number(year()),
                 arabicDateString);
    sublabel.priority = CalendarEvents::CalendarEventsPlugin::SubLabelPriority::Low;

    return sublabel;
}

IslamicCalendarProvider::IslamicCalendarProvider(QObject *parent,
                                                 CalendarSystem::System calendarSystem,
                                                 std::vector<QDate> &&alternateDates,
                                                 std::vector<QDate> &&sublabelDates,
                                                 int dateOffset)
    : AbstractCalendarProvider(parent, calendarSystem, std::move(alternateDates), std::move(sublabelDates), dateOffset)
    , d(std::make_unique<IslamicCalendarProviderPrivate>(calendarSystem))
{
    Q_ASSERT(m_calendarSystem == CalendarSystem::Jalali || m_calendarSystem == CalendarSystem::Islamic || m_calendarSystem == CalendarSystem::IslamicCivil
             || m_calendarSystem == CalendarSystem::IslamicUmalqura);
}

IslamicCalendarProvider::~IslamicCalendarProvider()
{
}

QCalendar::YearMonthDay IslamicCalendarProvider::fromGregorian(const QDate &date) const
{
    return d->fromGregorian(date);
}

CalendarEvents::CalendarEventsPlugin::SubLabel IslamicCalendarProvider::subLabel(const QDate &date) const
{
    return d->subLabel(date);
}
