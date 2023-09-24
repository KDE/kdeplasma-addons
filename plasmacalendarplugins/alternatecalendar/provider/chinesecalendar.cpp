/*
    SPDX-FileCopyrightText: 2021 Gary Wang <wzc782970009@gmail.com>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "chinesecalendar.h"

#include "solarutils.h"

namespace
{
const QStringList s_solarTermNames{
    QStringLiteral("春分"), QStringLiteral("清明"), QStringLiteral("谷雨"), QStringLiteral("立夏"), QStringLiteral("小满"), QStringLiteral("芒种"),
    QStringLiteral("夏至"), QStringLiteral("小暑"), QStringLiteral("大暑"), QStringLiteral("立秋"), QStringLiteral("处暑"), QStringLiteral("白露"),
    QStringLiteral("秋分"), QStringLiteral("寒露"), QStringLiteral("霜降"), QStringLiteral("立冬"), QStringLiteral("小雪"), QStringLiteral("大雪"),
    QStringLiteral("冬至"), QStringLiteral("小寒"), QStringLiteral("大寒"), QStringLiteral("立春"), QStringLiteral("雨水"), QStringLiteral("惊蛰"),
};
}

class ChineseCalendarProviderPrivate : public ICUCalendarPrivate
{
public:
    explicit ChineseCalendarProviderPrivate();

    CalendarEvents::CalendarEventsPlugin::SubLabel subLabel(const QDate &date);

private:
    enum SolarTerm {
        ChunFen = 0,
        QingMing,
        GuYu,
        LiXia,
        XiaoMan,
        MangZhong,
        XiaZhi,
        XiaoShu,
        DaShu,
        LiQiu,
        ChuShu,
        BaiLu,
        QiuFen,
        HanLu,
        ShuangJiang,
        LiDong,
        XiaoXue,
        DaXue,
        DongZhi,
        XiaoHan,
        DaHan,
        LiChun,
        YuShui,
        JingZhe,
    };

    /**
     * For formatting, see the documentation of SimpleDateFormat:
     * https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1SimpleDateFormat.html#details
     */
    QString formattedDateString(const icu::UnicodeString &str, bool hanidays = false) const;

    QString yearDisplayName() const;
    QString monthDisplayName() const;
    QString dayDisplayName() const;

    auto generateSolarTermsCache(int year);

    /**
     * Calculates Julian day of a solar term based on Newton's method
     */
    QDate getSolarTermDate(int year, SolarTerm order) const;

    static QHash<int /*year*/, std::array<QDate, 25>> s_solarTermsMap;

    icu::Locale m_locale;
    icu::Locale m_hanidaysLocale;
};

QHash<int, std::array<QDate, 25>> ChineseCalendarProviderPrivate::s_solarTermsMap = QHash<int, std::array<QDate, 25>>();

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

auto ChineseCalendarProviderPrivate::generateSolarTermsCache(int year)
{
#if __has_cpp_attribute(assume)
    [[assume(year > 0)]];
#endif
    auto thisYearIt = s_solarTermsMap.find(year);
    if (thisYearIt != s_solarTermsMap.end()) {
        // Already generated
        return thisYearIt;
    }

    thisYearIt = s_solarTermsMap.insert(year, {});

    SolarTerm solarTermIndex = DongZhi;
    for (int i = 0; i < 25; i++) {
        (*thisYearIt)[i] = getSolarTermDate(year - 1, solarTermIndex);
        if (solarTermIndex == DongZhi) {
            year++;
        }
        solarTermIndex = static_cast<SolarTerm>((solarTermIndex + 1) % 24);
    }

    return thisYearIt;
}

CalendarEvents::CalendarEventsPlugin::SubLabel ChineseCalendarProviderPrivate::subLabel(const QDate &date)
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};

    if (U_FAILURE(m_errorCode) || !date.isValid() || !setDate(date)) {
        return sublabel;
    }

    sublabel.yearLabel = yearDisplayName();
    sublabel.monthLabel = monthDisplayName();

    // Check solar term cache exists
    auto it = s_solarTermsMap.find(date.year());
    if (it == s_solarTermsMap.end()) {
        it = generateSolarTermsCache(date.year());
    }

    int solarTermIndex = -1;
    {
        const int indexInList = 2 * date.month() - 1;
        const QDate date1 = it->at(indexInList);
        const QDate date2 = it->at(indexInList + 1);
        if (date1.day() == date.day()) {
            solarTermIndex = (indexInList + DongZhi) % 24;
        } else if (date2.day() == date.day()) {
            solarTermIndex = (indexInList + 1 + DongZhi) % 24;
        }
    }

    const QString dayName = dayDisplayName();
    sublabel.dayLabel = day() == 1 ? monthDisplayName() : (solarTermIndex >= 0 ? s_solarTermNames.at(solarTermIndex) : dayName);
    const QString solarTerm = solarTermIndex >= 0 ? QStringLiteral(" (%1)").arg(s_solarTermNames.at(solarTermIndex)) : QString();
    sublabel.label = QStringLiteral("%1%2%3%4").arg(sublabel.yearLabel, sublabel.monthLabel, dayName, solarTerm);
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

QDate ChineseCalendarProviderPrivate::getSolarTermDate(int year, SolarTerm order) const
{
#if __has_cpp_attribute(assume)
    [[assume(year > 0)]];
#endif
    constexpr double RADIANS_PER_TERM = std::numbers::pi / 12.0;
    double angle = double(order) * RADIANS_PER_TERM;
    int month = ((order + 1) / 2 + 2) % 12 + 1;
    // 春分: Mar 20th
    int day = 6;
    if (order % 2 == 0) {
        day = 20;
    }
    const std::int64_t initialJulianDay = SolarUtils::toJulianDay(year, month, day);
    // Can't use QDate::toJulianDay because it doesn't support extra hours
    double julianDay = SolarUtils::NewtonIteration(angle, initialJulianDay);
    // To UTC+8 time
    julianDay += 8.0 / 24.0;

    int resultYear, resultMonth, resultDay;
    SolarUtils::getDateFromJulianDay(julianDay, resultYear, resultMonth, resultDay);
    //  TT -> UTC
    julianDay -= SolarUtils::getDeltaT(resultYear, resultMonth) / 86400;
    SolarUtils::getDateFromJulianDay(julianDay, resultYear, resultMonth, resultDay);

    return QDate(resultYear, resultMonth, resultDay);
}

ChineseCalendarProvider::ChineseCalendarProvider(QObject *parent,
                                                 CalendarSystem::System calendarSystem,
                                                 std::vector<QDate> &&alternateDates,
                                                 std::vector<QDate> &&sublabelDates)
    : AbstractCalendarProvider(parent, calendarSystem, std::move(alternateDates), std::move(sublabelDates))
    , d(std::make_unique<ChineseCalendarProviderPrivate>())
{
    Q_ASSERT(m_calendarSystem == CalendarSystem::Chinese);
}

ChineseCalendarProvider::~ChineseCalendarProvider()
{
}

CalendarEvents::CalendarEventsPlugin::SubLabel ChineseCalendarProvider::subLabel(const QDate &date) const
{
    return d->subLabel(date);
}
