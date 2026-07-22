/*
    SPDX-FileCopyrightText: 2026 Trần Nam Tuấn <tuantran1632001@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "vietnamesecalendar.h"

#include "solarutils.h"

#include <KLocalizedString>

#include <mutex>
#include <shared_mutex>

using namespace Qt::StringLiterals;

/*
    Vietnamese lunar calendar implementation.

    This work is technically duplicated with ICU Chinese calendar.
    However, ICU public API does not allow changing the timezone used for calcuation.
    This code should be revised once upstream provides support Vietnamese calendar.

    The algorithm used is based on Hồ Ngọc Đức's work.
    It has been adapted to use what available in `solarutils.h`.
    While providing greater accuracy, it is (in theory) more computational expensive.

    Although, the timezone is hardcoded in this algo,
    it technically could calculate for other timezones as well.
    E.g. Vietnamese is UTC+7, while Chinese is UTC+8.

    References:
    - https://unicode-org.atlassian.net/browse/ICU-11973
    - https://www.xemamlich.uhm.vn/calrules.html
 */

namespace
{
/*!
    \brief The interval of a lunar month.

    Or, the distant between two New Moon dates (ngày Sóc).

    \internal
 */
constexpr double s_newMoonInterval = 29.530588853;

/*!
    \brief The Julian Date of Jan 6, 2000 New Moon.

    \internal
 */
constexpr double s_J2000NewMoon = 2451550.09766;

/*!
 * \brief The UTC timezone offset of Vietnam.
 *
 * \internal
 */
constexpr int s_vietnameseUtcOffset = 7;

/*!
 * \brief The 10 heavenly stems in Vietnamese.
 *
 * \internal
 */
const std::array s_heavenlyStems{u"Giáp"_s, u"Ất"_s, u"Bính"_s, u"Đinh"_s, u"Mậu"_s, u"Kỷ"_s, u"Canh"_s, u"Tân"_s, u"Nhâm"_s, u"Quý"_s};

/*!
 * \brief The 12 earthly branches in Vietnamese.
 *
 * \internal
 */
const std::array s_earthlyBranches{u"Tý"_s, u"Sửu"_s, u"Dần"_s, u"Mão"_s, u"Thìn"_s, u"Tỵ"_s, u"Ngọ"_s, u"Mùi"_s, u"Thân"_s, u"Dậu"_s, u"Tuất"_s, u"Hợi"_s};

struct LunarDate {
    int year;
    int month;
    int day;
    bool isLeapMonth;
};

std::shared_mutex s_cacheMutex;
constinit QHash<int /* k-index */, double> s_winterSolsticeCache;
constinit QHash<int /* k-index */, double> s_newMoonCache;
constinit QHash<QDate, LunarDate> s_lunarDateCache;
constinit QHash<int /* lunar year */, QString> s_ganzhiYearCache;

/*!
 * \brief Get the local midnight UTC+7 for \a{dayIndex}.
 *
 * \internal
 */
double getLocalMidnight(qint64 dayIndex)
{
    return double(dayIndex) - 0.5 // shifts backward half a day from noon UTC to midnight UTC (00:00 UTC)
        - double(s_vietnameseUtcOffset) / 24.0; // shifts backward a further 7 hours
}

/*!
    \brief Get the JDE for the might of \a{dayIndex}.
    \internal
 */
double getMidnightJDE(qint64 dayIndex)
{
    double jdUtc = getLocalMidnight(dayIndex);

    int y, m, d;
    SolarUtils::getDateFromJulianDay(jdUtc, y, m, d);
    double dt = SolarUtils::getDeltaT(y, m) / 86400.0;

    return jdUtc + dt;
}

/*!
    \brief Returns the localized Gregorian day index of Julian Ephemeris Date \a{jde}.
    \internal
 */
qint64 getLocalDayIndex(double jde)
{
    int year, month, day;
    SolarUtils::getDateFromJulianDay(jde, year, month, day);
    double dt = SolarUtils::getDeltaT(year, month) / 86400.0;
    double jdUtc = jde - dt;
    // Vietnamese is UTC+7
    return std::floor(jdUtc + double(s_vietnameseUtcOffset) / 24.0 + 0.5);
}

/*!
    \brief Calculate the exact moment of a New Moon day (ngày Sóc) from \a{approxJde}.

    This method use Newton's method to find the value. A New Moon day starts when:
    ```
        f(t) = MoonLongitude(t) - SunLongitude(t) = 0
    ```

    \internal
 */
double getNewMoon(double approxJde)
{
    double x = approxJde;
    constexpr double EPSILON = 1e-7;
    constexpr double DELTA = 5e-6;

    constexpr auto func = [](double jd) -> double {
        double sunL = SolarUtils::getEarthEclipticLongitudeForSun(jd);
        double moonL = SolarUtils::getMoonEclipticLongitudeEC(jd);
        double diff = moonL - sunL;
        // Normalize to [-PI, PI]
        while (diff < -std::numbers::pi) {
            diff += 2 * std::numbers::pi;
        }
        while (diff > std::numbers::pi) {
            diff -= 2 * std::numbers::pi;
        }
        return diff;
    };

    for (int i = 0; i < 100; ++i) {
        double fx = func(x);
        if (std::abs(fx) <= EPSILON) {
            break;
        }
        double fpx = (func(x + DELTA) - func(x - DELTA)) / (2 * DELTA);

        // Fallback to mean relative velocity if derivative is too small
        if (std::abs(fpx) < 0.1) {
            // The mean relative angular velocity of the Moon relative to the Sun (radians/day).
            // Or, the speed of the Moon revolving around the Sun in 1 lunar month.
            fpx = 2 * std::numbers::pi / s_newMoonInterval;
        }
        x = x - fx / fpx;
    }
    return x;
}

/*!
    \brief Returns the JDE of the \a{k}-th New Moon since the J2000 epoch.

    Ideally, this method should be updated in the far future to use a new epoch to avoid drifting.
    A negative \a{k} index is considered valid. Care must be taken to maintain this requirement.

    The computed value is cached.

    \internal
 */
double getNewMoonByIndex(int k)
{
    {
        std::shared_lock lock(s_cacheMutex);
        if (s_newMoonCache.contains(k)) {
            return s_newMoonCache.value(k);
        }
    }

    const auto value = getNewMoon(s_J2000NewMoon + k * s_newMoonInterval);

    {
        std::unique_lock lock(s_cacheMutex);
        s_newMoonCache.insert(k, value);
    }

    return value;
}

/*!
    \brief Find the precise JDE for the Winter Solstice (Đông chí) of a given \a{year}.

    A Winter Solstice occurs when the Sun is at 270 degrees. It's typically around 19 to 22 in December.

    The computed value is cached.

    \internal
 */
double getWinterSolsticeJDE(int year)
{
    {
        std::shared_lock lock(s_cacheMutex);
        if (s_winterSolsticeCache.contains(year)) {
            return s_winterSolsticeCache.value(year);
        }
    }

    const double approxJde = SolarUtils::toJulianDay(year, 12, 21);
    const auto value = SolarUtils::NewtonIteration(270.0 * std::numbers::pi / 180.0, approxJde);

    {
        std::unique_lock lock(s_cacheMutex);
        s_winterSolsticeCache.insert(year, value);
    }

    return value;
}

/*!
    \brief Evaluates if a lunar month contains a Major Solar Term (Zhongqi, or, Trung khí) from \a{jdeStart} to \a{jdeEnd}.
    \internal
 */
bool hasMajorSolarTerm(double jdeStart, double jdeEnd)
{
    // Get the precise midnight time
    const auto midnightJdeStart = getMidnightJDE(getLocalDayIndex(jdeStart));
    const auto midnightJdeEnd = getMidnightJDE(getLocalDayIndex(jdeEnd));

    double longitudeStart = SolarUtils::getEarthEclipticLongitudeForSun(midnightJdeStart) * 180.0 / std::numbers::pi;
    double longitudeEnd = SolarUtils::getEarthEclipticLongitudeForSun(midnightJdeEnd) * 180.0 / std::numbers::pi;

    if (longitudeEnd <= longitudeStart) {
        longitudeEnd += 360.0;
    }

    // A Zhongqi (0, 30, 60...) exists if a multiple of 30 is in [l_start, l_end)
    constexpr auto EPSILON = 1e-8;
    double z = std::ceil((longitudeStart - EPSILON) / 30.0) * 30.0;
    return z < (longitudeEnd - EPSILON);
}

/*!
    \brief Find the lunar month index containing the given Julian day \a{dayIndex}.
    \internal
 */
int getLocalMonthIndex(qint64 dayIndex)
{
    int k = std::floor((getLocalMidnight(dayIndex) - s_J2000NewMoon) / s_newMoonInterval);
    // Align k perfectly so it points to the month block the day belongs to.
    while (getLocalDayIndex(getNewMoonByIndex(k + 1)) <= dayIndex) {
        k++;
    }
    while (getLocalDayIndex(getNewMoonByIndex(k)) > dayIndex) {
        k--;
    }
    return k;
}

/*!
    \brief Get the lunar date of \a{date}.
    \internal
 */
LunarDate getLunarDate(const QDate &date)
{
    {
        std::shared_lock lock(s_cacheMutex);
        if (s_lunarDateCache.contains(date)) {
            return s_lunarDateCache[date];
        }
    }

    int day = date.day();
    int month = date.month();
    int year = date.year();

    auto jd = SolarUtils::toJulianDay(year, month, day);

    // 1. Get the month index for the target date.
    auto k = getLocalMonthIndex(jd);

    // 2. Locate the framing Winter Solstices.
    int k_wsBefore, k_wsAfter, lunarYear;
    {
        auto k_wsCurr = getLocalMonthIndex(getWinterSolsticeJDE(year));
        if (k >= k_wsCurr) {
            k_wsBefore = k_wsCurr;
            k_wsAfter = getLocalMonthIndex(getWinterSolsticeJDE(year + 1));
            lunarYear = year;
        } else {
            k_wsBefore = getLocalMonthIndex(getWinterSolsticeJDE(year - 1));
            k_wsAfter = k_wsCurr;
            lunarYear = year - 1;
        }
    }

    // 3. Assess if a Leap Month exists in this Solstice-to-Solstice cycle
    bool isLeapMonth = false;
    bool isLeapYear = k_wsAfter - k_wsBefore == 13; // Leap year has 13 months
    int k_leap = std::numeric_limits<int>::min();
    if (isLeapYear) {
        for (int i = k_wsBefore + 1; i < k_wsAfter; ++i) {
            if (!hasMajorSolarTerm(getNewMoonByIndex(i), getNewMoonByIndex(i + 1))) {
                k_leap = i; // First month without a Zhongqi becomes the leap month
                break;
            }
        }
        isLeapMonth = (k == k_leap);
    }

    // 4. Calculate final Lunar Date
    int leapOffset = (isLeapYear && k > k_leap) ? 1 : 0;
    int lunarMonth = ((k - k_wsBefore - leapOffset) + 10) % 12;
    if (!isLeapMonth) { // Leap month use the same number as the previous month.
        lunarMonth++;
    } else if (lunarMonth == 0) {
        lunarMonth = 12; // wrap 0 to 12 for leap month 12, astronomically rare but not impossible
    }

    if (lunarMonth < 11) { // Month before 11 belong to the following year in this calculation.
        lunarYear++;
    }

    int lunarDay = jd - getLocalDayIndex(getNewMoonByIndex(k)) + 1;

    LunarDate lunarDate{lunarYear, lunarMonth, lunarDay, isLeapMonth};

    {
        std::unique_lock lock(s_cacheMutex);
        s_lunarDateCache[date] = lunarDate;
    }

    return lunarDate;
}

/*!
   \brief Get the ganzhi name for \a{year}.
   \internal
 */
QString getYearGanzhi(int year)
{
    {
        std::shared_lock lock(s_cacheMutex);
        if (s_ganzhiYearCache.contains(year)) {
            return s_ganzhiYearCache[year];
        }
    }
    return s_ganzhiYearCache[year] = s_heavenlyStems.at((year + 6) % 10) + ' '_L1 + s_earthlyBranches.at((year + 8) % 12);
}
}

class VietnameseCalendarProviderPrivate
{
    Q_DISABLE_COPY(VietnameseCalendarProviderPrivate)
public:
    VietnameseCalendarProviderPrivate() = default;

    QCalendar::YearMonthDay fromGregorian(const QDate &date);
    CalendarEvents::CalendarEventsPlugin::SubLabel subLabel(const QDate &date);

private:
    QDate m_date;
    LunarDate m_lunarDate{};

    bool setDate(const QDate &date);
};

bool VietnameseCalendarProviderPrivate::setDate(const QDate &date)
{
    if (!date.isValid()) {
        return false;
    }
    m_date = date;
    m_lunarDate = getLunarDate(date);
    return true;
}

QCalendar::YearMonthDay VietnameseCalendarProviderPrivate::fromGregorian(const QDate &date)
{
    if (!setDate(date)) {
        return {};
    }
    return {m_lunarDate.year, m_lunarDate.month, m_lunarDate.year};
}

CalendarEvents::CalendarEventsPlugin::SubLabel VietnameseCalendarProviderPrivate::subLabel(const QDate &date)
{
    if (!setDate(date)) {
        return {};
    }

    CalendarEvents::CalendarEventsPlugin::SubLabel sublabel;
    sublabel.priority = CalendarEvents::CalendarEventsPlugin::SubLabelPriority::Low;

    sublabel.dayLabel = QString::number(m_lunarDate.day);
    sublabel.monthLabel = QString::number(m_lunarDate.month);
    if (m_lunarDate.isLeapMonth) {
        sublabel.monthLabel.append(u" Nhuận"_s);
    }
    sublabel.yearLabel = getYearGanzhi(m_lunarDate.year).append(' '_L1).append(QString::number(m_lunarDate.year));
    sublabel.label = u"Ngày %1 tháng %2 năm %3"_s.arg(sublabel.dayLabel, sublabel.monthLabel, sublabel.yearLabel);

    return sublabel;
}

VietnameseCalendarProvider::VietnameseCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem, const QDate &startDate, const QDate &endDate)
    : AbstractCalendarProvider{parent, calendarSystem, startDate, endDate}
    , d{std::make_unique<VietnameseCalendarProviderPrivate>()}
{
    Q_ASSERT(calendarSystem == CalendarSystem::System::Vietnamese);
}

VietnameseCalendarProvider::~VietnameseCalendarProvider() = default;

QCalendar::YearMonthDay VietnameseCalendarProvider::fromGregorian(const QDate &date) const
{
    return d->fromGregorian(date);
}

CalendarEvents::CalendarEventsPlugin::SubLabel VietnameseCalendarProvider::subLabel(const QDate &date) const
{
    return d->subLabel(date);
}
