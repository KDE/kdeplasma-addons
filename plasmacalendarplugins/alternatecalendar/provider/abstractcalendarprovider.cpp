/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "abstractcalendarprovider.h"

AbstractCalendarProvider::AbstractCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem)
    : QObject(parent)
    , m_calendarSystem(calendarSystem)
{
}

AbstractCalendarProvider::~AbstractCalendarProvider()
{
}

QDate AbstractCalendarProvider::fromGregorian(const QDate &date) const
{
    if (!date.isValid()) {
        return QDate();
    }

    return date;
}

CalendarEvents::CalendarEventsPlugin::SubLabel AbstractCalendarProvider::subLabels([[maybe_unused]] const QDate &date) const
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};
    sublabel.priority = CalendarEvents::CalendarEventsPlugin::SubLabelPriority::Low;

    return sublabel;
}
