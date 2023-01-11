/*
    SPDX-FileCopyrightText: 2021 Gary Wang <wzc782970009@gmail.com>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "chinesecalendar.h"

class ChineseCalendarProviderPrivate : public ICUCalendarPrivate
{
public:
    explicit ChineseCalendarProviderPrivate();

    CalendarEvents::CalendarEventsPlugin::SubLabel subLabels(const QDate &date);

private:
    /**
     * For formatting, see the documentation of SimpleDateFormat:
     * https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1SimpleDateFormat.html#details
     */
    QString formattedDateString(const icu::UnicodeString &str, bool hanidays = false) const;

    QString yearDisplayName() const;
    QString monthDisplayName() const;
    QString dayDisplayName() const;

    icu::Locale m_locale;
    icu::Locale m_hanidaysLocale;
};

ChineseCalendarProviderPrivate::ChineseCalendarProviderPrivate()
    : ICUCalendarPrivate()
    , m_locale(icu::Locale("zh", 0, 0, "calendar=chinese"))
    , m_hanidaysLocale(icu::Locale("zh", 0, 0, "calendar=chinese;numbers=hanidays"))
{
    if (U_FAILURE(m_errorCode)) {
        return; // Failed to create m_GregorianCalendar
    }

    m_calendar.reset(icu::Calendar::createInstance("en_US@calendar=chinese", m_errorCode));
}

CalendarEvents::CalendarEventsPlugin::SubLabel ChineseCalendarProviderPrivate::subLabels(const QDate &date)
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};

    if (U_FAILURE(m_errorCode) || !date.isValid() || !setDate(date)) {
        return sublabel;
    }

    sublabel.yearLabel = yearDisplayName();
    sublabel.monthLabel = monthDisplayName();
    const QString dayName = dayDisplayName();
    sublabel.dayLabel = day() == 1 ? monthDisplayName() : dayName;
    sublabel.label = QStringLiteral("%1%2%3").arg(sublabel.yearLabel, sublabel.monthLabel, dayName);
    sublabel.priority = CalendarEvents::CalendarEventsPlugin::SubLabelPriority::Low;

    return sublabel;
}

QString ChineseCalendarProviderPrivate::formattedDateString(const icu::UnicodeString &str, bool hanidays) const
{
    UErrorCode errorCode = U_ZERO_ERROR;
    icu::UnicodeString dateString;
    icu::SimpleDateFormat formatter(str, hanidays ? m_hanidaysLocale : m_locale, errorCode);
    formatter.setCalendar(*m_calendar);
    formatter.format(m_calendar->getTime(errorCode), dateString);

    std::string utf8Str;
    dateString.toUTF8String<std::string>(utf8Str);

    return QString::fromUtf8(utf8Str.c_str());
}

QString ChineseCalendarProviderPrivate::yearDisplayName() const
{
    return formattedDateString("U");
}

QString ChineseCalendarProviderPrivate::monthDisplayName() const
{
    return formattedDateString("MMM");
}

QString ChineseCalendarProviderPrivate::dayDisplayName() const
{
    return formattedDateString("d", true);
}

ChineseCalendarProvider::ChineseCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem)
    : AbstractCalendarProvider(parent, calendarSystem)
    , d(std::make_unique<ChineseCalendarProviderPrivate>())
{
    Q_ASSERT(m_calendarSystem == CalendarSystem::Chinese);
}

ChineseCalendarProvider::~ChineseCalendarProvider()
{
}

CalendarEvents::CalendarEventsPlugin::SubLabel ChineseCalendarProvider::subLabels(const QDate &date) const
{
    return d->subLabels(date);
}
