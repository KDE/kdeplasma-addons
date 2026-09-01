/*
    SPDX-FileCopyrightText: 2026 Satya Prakash Dahal

    SPDX-License-Identifier: GPL-2.0-or-later

    Date mapping data taken from the official Kathmandu Metropolitan City calendar:
    https://new.kathmandu.gov.np/en/calendar
    Day counts per month for BS years 2000-2099 and their corresponding
    Gregorian start dates (Baishakh 1) are sourced from that site.
 */

#include "nepalicalendar.h"

#include <KLocalizedString>

#include <array>
#include <cmath>
#include <numeric>

using namespace Qt::StringLiterals;

namespace
{
constexpr int s_firstBsYear = 2000;
constexpr std::size_t s_yearCount = 100; // BS 2000 - BS 2099

constexpr std::array<std::array<quint8, 12>, s_yearCount> s_daysPerMonth{{
    {30, 32, 31, 32, 31, 30, 30, 30, 29, 30, 29, 31}, // BS 2000
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2001
    {31, 31, 32, 32, 31, 30, 30, 29, 30, 29, 30, 30}, // BS 2002
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2003
    {30, 32, 31, 32, 31, 30, 30, 30, 29, 30, 29, 31}, // BS 2004
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2005
    {31, 31, 32, 32, 31, 30, 30, 29, 30, 29, 30, 30}, // BS 2006
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2007
    {31, 31, 31, 32, 31, 31, 29, 30, 30, 29, 29, 31}, // BS 2008
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2009
    {31, 31, 32, 32, 31, 30, 30, 29, 30, 29, 30, 30}, // BS 2010
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2011
    {31, 31, 31, 32, 31, 31, 29, 30, 30, 29, 30, 30}, // BS 2012
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2013
    {31, 31, 32, 32, 31, 30, 30, 29, 30, 29, 30, 30}, // BS 2014
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2015
    {31, 31, 31, 32, 31, 31, 29, 30, 30, 29, 30, 30}, // BS 2016
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2017
    {31, 32, 31, 32, 31, 30, 30, 29, 30, 29, 30, 30}, // BS 2018
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 30, 29, 31}, // BS 2019
    {31, 31, 31, 32, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2020
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2021
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 30}, // BS 2022
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 30, 29, 31}, // BS 2023
    {31, 31, 31, 32, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2024
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2025
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2026
    {30, 32, 31, 32, 31, 30, 30, 30, 29, 30, 29, 31}, // BS 2027
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2028
    {31, 31, 32, 31, 32, 30, 30, 29, 30, 29, 30, 30}, // BS 2029
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2030
    {30, 32, 31, 32, 31, 30, 30, 30, 29, 30, 29, 31}, // BS 2031
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2032
    {31, 31, 32, 32, 31, 30, 30, 29, 30, 29, 30, 30}, // BS 2033
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2034
    {30, 32, 31, 32, 31, 31, 29, 30, 30, 29, 29, 31}, // BS 2035
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2036
    {31, 31, 32, 32, 31, 30, 30, 29, 30, 29, 30, 30}, // BS 2037
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2038
    {31, 31, 31, 32, 31, 31, 29, 30, 30, 29, 30, 30}, // BS 2039
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2040
    {31, 31, 32, 32, 31, 30, 30, 29, 30, 29, 30, 30}, // BS 2041
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2042
    {31, 31, 31, 32, 31, 31, 29, 30, 30, 29, 30, 30}, // BS 2043
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2044
    {31, 32, 31, 32, 31, 30, 30, 29, 30, 29, 30, 30}, // BS 2045
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2046
    {31, 31, 31, 32, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2047
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2048
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 30}, // BS 2049
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 30, 29, 31}, // BS 2050
    {31, 31, 31, 32, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2051
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2052
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 30}, // BS 2053
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 30, 29, 31}, // BS 2054
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2055
    {31, 31, 32, 31, 32, 30, 30, 29, 30, 29, 30, 30}, // BS 2056
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2057
    {30, 32, 31, 32, 31, 30, 30, 30, 29, 30, 29, 31}, // BS 2058
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2059
    {31, 31, 32, 32, 31, 30, 30, 29, 30, 29, 30, 30}, // BS 2060
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2061
    {31, 31, 31, 32, 31, 31, 29, 30, 29, 30, 29, 31}, // BS 2062
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2063
    {31, 31, 32, 32, 31, 30, 30, 29, 30, 29, 30, 30}, // BS 2064
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2065
    {31, 31, 31, 32, 31, 31, 29, 30, 30, 29, 29, 31}, // BS 2066
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2067
    {31, 31, 32, 32, 31, 30, 30, 29, 30, 29, 30, 30}, // BS 2068
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2069
    {31, 31, 31, 32, 31, 31, 29, 30, 30, 29, 30, 30}, // BS 2070
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2071
    {31, 32, 31, 32, 31, 30, 30, 29, 30, 29, 30, 30}, // BS 2072
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 31}, // BS 2073
    {31, 31, 31, 32, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2074
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2075
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 30}, // BS 2076
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 30, 29, 31}, // BS 2077
    {31, 31, 31, 32, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2078
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2079
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 29, 30, 30}, // BS 2080
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 30, 29, 31}, // BS 2081
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2082
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 29, 30, 30}, // BS 2083
    {31, 31, 32, 31, 31, 30, 30, 30, 29, 30, 30, 30}, // BS 2084
    {31, 32, 31, 32, 30, 31, 30, 30, 29, 30, 30, 30}, // BS 2085
    {30, 32, 31, 32, 31, 30, 30, 30, 29, 30, 30, 30}, // BS 2086
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 30, 30, 30}, // BS 2087
    {30, 31, 32, 32, 30, 31, 30, 30, 29, 30, 30, 30}, // BS 2088
    {30, 32, 31, 32, 31, 30, 30, 30, 29, 30, 30, 30}, // BS 2089
    {30, 32, 31, 32, 31, 30, 30, 30, 29, 30, 30, 30}, // BS 2090
    {31, 31, 32, 31, 31, 31, 30, 30, 29, 30, 30, 30}, // BS 2091
    {30, 31, 32, 32, 31, 30, 30, 30, 29, 30, 30, 30}, // BS 2092
    {30, 32, 31, 32, 31, 30, 30, 30, 29, 30, 30, 30}, // BS 2093
    {31, 31, 32, 31, 31, 30, 30, 30, 29, 30, 30, 30}, // BS 2094
    {31, 31, 32, 31, 31, 31, 30, 29, 30, 30, 30, 30}, // BS 2095
    {30, 31, 32, 32, 31, 30, 30, 29, 30, 29, 30, 30}, // BS 2096
    {31, 32, 31, 32, 31, 30, 30, 30, 29, 30, 30, 30}, // BS 2097
    {31, 31, 32, 31, 31, 31, 29, 30, 29, 30, 29, 31}, // BS 2098
    {31, 31, 32, 31, 31, 31, 30, 29, 29, 30, 30, 30}, // BS 2099
}};

// Days between Baishakh 1 of BS 2000 (epoch) and Baishakh 1 of each BS year.
// These are the actual Gregorian start dates from the KMC data.
constexpr std::array<int, s_yearCount> s_yearStartDays{
    0,     365,   730,   1095,  1461,  1826,  2191,  2556,  2922,  3287,  3652,  4017,  4383,  4748,  5113,  5478,  5844,  6209,  6574,  6939,
    7305,  7670,  8035,  8400,  8766,  9131,  9496,  9862,  10227, 10592, 10957, 11323, 11688, 12053, 12418, 12784, 13149, 13514, 13879, 14245,
    14610, 14975, 15340, 15706, 16071, 16436, 16801, 17167, 17532, 17897, 18262, 18628, 18993, 19358, 19723, 20089, 20454, 20819, 21185, 21550,
    21915, 22280, 22646, 23011, 23376, 23741, 24107, 24472, 24837, 25202, 25568, 25933, 26298, 26663, 27029, 27394, 27759, 28124, 28490, 28855,
    29220, 29585, 29951, 30316, 30681, 31046, 31412, 31777, 32143, 32508, 32873, 33238, 33604, 33969, 34334, 34699, 35065, 35429, 35795, 36160,
};

// Total number of days covered by the table (BS 2000-01-01 to BS 2099-12-30).
constexpr int s_totalDays =
    s_yearStartDays[s_yearCount - 1] + std::accumulate(s_daysPerMonth[s_yearCount - 1].begin(), s_daysPerMonth[s_yearCount - 1].end(), 0);

// Mean month length derived from the table, used to approximate dates outside it.
constexpr double s_meanMonthDays = static_cast<double>(s_totalDays) / (s_yearCount * 12);

// Baishakh 1 of BS 2000.
const QDate s_epoch(1943, 4, 14);

const std::array<QString, 12> s_devanagariMonths{
    u"बैशाख"_s,
    u"जेठ"_s,
    u"असार"_s,
    u"साउन"_s,
    u"भदौ"_s,
    u"आश्विन"_s,
    u"कार्तिक"_s,
    u"मंसिर"_s,
    u"पुष"_s,
    u"माघ"_s,
    u"फाल्गुण"_s,
    u"चैत्र"_s,
};

QString toNepaliNumber(int number)
{
    static const QString digits = u"०१२३४५६७८९"_s;
    QString result;
    result.reserve(4);
    for (const QChar c : QString::number(number)) {
        result.append(digits.at(c.digitValue()));
    }
    return result;
}

struct BsDate {
    int year;
    int month;
    int day;
    bool inTable;
};

BsDate toBsDate(const QDate &date)
{
    const qint64 days = s_epoch.daysTo(date);

    if (days >= 0 && days < s_totalDays) {
        // Find the BS year whose [start, nextStart) range contains the date, matching
        // the reference implementation (the first matching year wins).
        std::size_t i = 0;
        while (i + 1 < s_yearCount && days >= s_yearStartDays[i + 1]) {
            ++i;
        }

        const int year = s_firstBsYear + static_cast<int>(i);
        int offset = static_cast<int>(days) - s_yearStartDays[i];
        int month = 0;
        while (month < 12 && offset >= s_daysPerMonth[i][month]) {
            offset -= s_daysPerMonth[i][month];
            ++month;
        }

        if (month < 12) {
            return {year, month + 1, offset + 1, true};
        }
    }

    // Outside the published table: approximate with the mean month length.
    const double monthCoord = static_cast<double>(days) / s_meanMonthDays;
    qint64 monthsTotal = std::floor(monthCoord);
    qint64 yearOffset = monthsTotal / 12;
    if (monthsTotal < 0 && monthsTotal % 12 != 0) {
        --yearOffset;
    }
    const int year = s_firstBsYear + static_cast<int>(yearOffset);
    const int month = static_cast<int>(monthsTotal - yearOffset * 12) + 1;
    const double fraction = static_cast<double>(days) - static_cast<double>(monthsTotal) * s_meanMonthDays;
    const int day = static_cast<int>(std::floor(fraction)) + 1;

    return {year, month, day, false};
}
}

NepaliCalendarProvider::NepaliCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem, const QDate &startDate, const QDate &endDate)
    : AbstractCalendarProvider(parent, calendarSystem, startDate, endDate)
{
    Q_ASSERT(m_calendarSystem == CalendarSystem::BikramSambat);
}

NepaliCalendarProvider::~NepaliCalendarProvider() = default;

QCalendar::YearMonthDay NepaliCalendarProvider::fromGregorian(const QDate &date) const
{
    if (!date.isValid()) {
        return {};
    }

    const BsDate bs = toBsDate(date);
    return {bs.year, bs.month, bs.day};
}

CalendarEvents::CalendarEventsPlugin::SubLabel NepaliCalendarProvider::subLabel(const QDate &date) const
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};
    sublabel.priority = CalendarEvents::CalendarEventsPlugin::SubLabelPriority::Low;

    if (!date.isValid()) {
        return sublabel;
    }

    const BsDate bs = toBsDate(date);
    const QString &dvMonthName = s_devanagariMonths[bs.month - 1];

    sublabel.dayLabel = QString::number(bs.day);
    sublabel.yearLabel = QString::number(bs.year);

    const QString approxSuffix =
        bs.inTable ? QString() : i18ndc("plasma_calendar_alternatecalendar", "@label date outside the officially published Bikram Sambat range", " (approx.)");
    sublabel.label = i18ndc("plasma_calendar_alternatecalendar",
                            "@label %1 Day number %2 Month name in Bikram Sambat %3 Year number",
                            "%1 %2, %3",
                            toNepaliNumber(bs.day),
                            dvMonthName,
                            toNepaliNumber(bs.year))
        + approxSuffix;

    return sublabel;
}

#include "moc_nepalicalendar.cpp"
