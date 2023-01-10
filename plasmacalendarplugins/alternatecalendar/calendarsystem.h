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
#if QT_CONFIG(jalalicalendar)
        Jalali = static_cast<int>(QCalendar::System::Jalali),
#endif
#if QT_CONFIG(islamiccivilcalendar)
        IslamicCivil = static_cast<int>(QCalendar::System::IslamicCivil),
#endif
        Chinese = static_cast<int>(QCalendar::System::Last) + 1, // QTBUG-89824: QCalendar doesn't support Chinese calendar system
        Indian, // QCalendar doesn't support India calendar system
        Hebrew,
    };
    Q_ENUM(System)
};
Q_DECLARE_METATYPE(CalendarSystem)

struct CalendarSystemItem {
    CalendarSystem::System system;
    QString id;
    QString text;
};

// clang-format off
static const std::map<QString /* id */, CalendarSystemItem> s_calendarMap{
    {
        QStringLiteral("Julian"),
        {CalendarSystem::Julian, QStringLiteral("Julian"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Julian")}
    },
    {
        QStringLiteral("Milankovic"),
        {CalendarSystem::Milankovic, QStringLiteral("Milankovic"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Milankovic")}
    },
    {
        QStringLiteral("Jalali"),
        {CalendarSystem::Jalali, QStringLiteral("Jalali"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "The Solar Hijri Calendar (Persian)")}
    },
    {
        QStringLiteral("IslamicCivil"),
        {CalendarSystem::IslamicCivil, QStringLiteral("IslamicCivil"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "The Islamic Civil Calendar")}
    },
    {
        QStringLiteral("Chinese"),
        {CalendarSystem::Chinese, QStringLiteral("Chinese"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Chinese Lunar Calendar")}
    },
    {
        QStringLiteral("Indian"),
        {CalendarSystem::Indian, QStringLiteral("Indian"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Indian National Calendar")}
    },
    {
        QStringLiteral("Hebrew"),
        {CalendarSystem::Hebrew, QStringLiteral("Hebrew"), i18ndc("plasma_calendar_alternatecalendar", "@item:inlist", "Hebrew Calendar")}
    },
};
// clang-format on
