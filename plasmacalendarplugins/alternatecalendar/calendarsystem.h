/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QCalendar>
#include <QMetaType>

class CalendarSystem
{
    Q_GADGET

public:
    /**
     * @note When adding a new calendar system, CalendarSystemModel should also be updated.
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
    };
    Q_ENUM(System)
};
Q_DECLARE_METATYPE(CalendarSystem)
