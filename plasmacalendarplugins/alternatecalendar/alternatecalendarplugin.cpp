/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "alternatecalendarplugin.h"

#include "provider/qtcalendar.h"

class AlternateCalendarPluginPrivate
{
public:
    explicit AlternateCalendarPluginPrivate(AlternateCalendarPlugin *parent);
    ~AlternateCalendarPluginPrivate();

    void init();
    AbstractCalendarProvider *calendarProvider() const;

    CalendarSystem::System m_calendarSystem;

private:
    std::unique_ptr<AbstractCalendarProvider> m_calendarProvider;

    AlternateCalendarPlugin *q;
};

AlternateCalendarPluginPrivate::AlternateCalendarPluginPrivate(AlternateCalendarPlugin *parent)
    : q(parent)
{
}

AlternateCalendarPluginPrivate::~AlternateCalendarPluginPrivate()
{
}

void AlternateCalendarPluginPrivate::init()
{
    // Load/Reload the calendar provider
    switch (m_calendarSystem) {
#ifndef QT_BOOTSTRAPPED
    case CalendarSystem::Julian:
    case CalendarSystem::Milankovic:
#endif
#if QT_CONFIG(jalalicalendar)
    case CalendarSystem::Jalali:
#endif
#if QT_CONFIG(islamiccivilcalendar)
    case CalendarSystem::IslamicCivil:
#endif
        m_calendarProvider.reset(new QtCalendarProvider(q, m_calendarSystem));
        break;
    default:
        m_calendarProvider.reset(new AbstractCalendarProvider(q, m_calendarSystem));
    }
}

AbstractCalendarProvider *AlternateCalendarPluginPrivate::calendarProvider() const
{
    return m_calendarProvider.get();
}

AlternateCalendarPlugin::AlternateCalendarPlugin(QObject *parent)
    : CalendarEvents::CalendarEventsPlugin(parent)
    , d(std::make_unique<AlternateCalendarPluginPrivate>(this))
{
}

AlternateCalendarPlugin::~AlternateCalendarPlugin()
{
}

void AlternateCalendarPlugin::loadEventsForDateRange(const QDate &startDate, const QDate &endDate)
{
    if (!endDate.isValid() || d->m_calendarSystem == CalendarSystem::Gregorian) {
        return;
    }

    QHash<QDate, QDate> alternateDatesData;
    QHash<QDate, SubLabel> subLabelsData;

    for (QDate date = startDate; date <= endDate && date.isValid(); date = date.addDays(1)) {
        if (const QDate alt = d->calendarProvider()->fromGregorian(date); alt != date) {
            alternateDatesData.insert(date, alt);
        }
        subLabelsData.insert(date, d->calendarProvider()->subLabels(date));
    }

    if (alternateDatesData.size() > 0) {
        Q_EMIT alternateDateReady(alternateDatesData);
    }
    Q_EMIT subLabelReady(subLabelsData);
}
