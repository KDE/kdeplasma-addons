/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <map>

#include <QCalendar>
#include <QMetaType>

#include <KLocalizedString>

class CalendarSystem
{
    Q_GADGET

public:
    /**
     * @note When adding a new calendar system, \s_calendarMap should also be updated.
     */
    enum System {
        Gregorian = static_cast<int>(QCalendar::System::Gregorian),
#ifndef QT_BOOTSTRAPPED
        Julian = static_cast<int>(QCalendar::System::Julian),
        Milankovic = static_cast<int>(QCalendar::System::Milankovic),
#endif
        Chinese = static_cast<int>(QCalendar::System::Last) + 1, // QTBUG-89824: QCalendar doesn't support Chinese calendar system
        Indian, // QCalendar doesn't support India calendar system
        Hebrew,
        Jalali, // Persian
        Islamic, // Astronomical
        IslamicCivil,
        IslamicUmalqura,
    };
    Q_ENUM(System)
};
Q_DECLARE_METATYPE(CalendarSystem)

struct CalendarSystemItem {
    CalendarSystemItem(CalendarSystem::System _system, const QString &_id, const QString &_text) = delete;
    constexpr CalendarSystemItem(CalendarSystem::System _system, const QStringView &_id, const QStringView &_text)
        : system(_system)
        , id(_id)
        , text(_text)
    {
    }

    const CalendarSystem::System system;
    const QStringView id;
    const QStringView text;
};

// clang-format off
static const std::map<QString /* id */, CalendarSystemItem> s_calendarMap{
    {
        QStringLiteral("Julian"),
        {CalendarSystem::Julian, QStringView(QStringLiteral("Julian")), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Julian")}
    },
    {
        QStringLiteral("Milankovic"),
        {CalendarSystem::Milankovic, QStringView(QStringLiteral("Milankovic")), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Milankovic")}
    },
    {
        QStringLiteral("Jalali"),
        {CalendarSystem::Jalali, QStringView(QStringLiteral("Jalali")), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "The Solar Hijri Calendar (Persian)")}
    },
    {
        QStringLiteral("Islamic"),
        {CalendarSystem::Islamic, QStringView(QStringLiteral("Islamic")), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist See https://cldr.unicode.org/development/development-process/design-proposals/islamic-calendar-types for more details", "Islamic Calendar (Astronomical)")}
    },
    {
        QStringLiteral("IslamicCivil"),
        {CalendarSystem::IslamicCivil, QStringView(QStringLiteral("IslamicCivil")), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist See https://cldr.unicode.org/development/development-process/design-proposals/islamic-calendar-types for more details", "The Islamic Civil Calendar (Tabular)")}
    },
    {
        QStringLiteral("IslamicUmalqura"),
        {CalendarSystem::IslamicUmalqura, QStringView(QStringLiteral("IslamicUmalqura")), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist See https://cldr.unicode.org/development/development-process/design-proposals/islamic-calendar-types for more details", "Islamic Calendar (Umm al-Qura)")}
    },
    {
        QStringLiteral("Chinese"),
        {CalendarSystem::Chinese, QStringView(QStringLiteral("Chinese")), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Chinese Lunar Calendar")}
    },
    {
        QStringLiteral("Indian"),
        {CalendarSystem::Indian, QStringView(QStringLiteral("Indian")), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Indian National Calendar")}
    },
    {
        QStringLiteral("Hebrew"),
        {CalendarSystem::Hebrew, QStringView(QStringLiteral("Hebrew")), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Hebrew Calendar")}
    },
};
// clang-format on
