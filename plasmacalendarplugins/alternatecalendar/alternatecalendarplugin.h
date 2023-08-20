/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ALTERNATECALENDARPLUGIN_H
#define ALTERNATECALENDARPLUGIN_H

#include <QCache>
#include <QDate>

#include <KConfigGroup>
#include <KConfigWatcher>

#include <CalendarEvents/CalendarEventsPlugin>

#include "calendarsystem.h"

class AlternateCalendarPlugin : public CalendarEvents::CalendarEventsPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.CalendarEventsPlugin" FILE "alternatecalendarplugin.json")
    Q_INTERFACES(CalendarEvents::CalendarEventsPlugin)

public:
    AlternateCalendarPlugin(QObject *parent = nullptr);
    ~AlternateCalendarPlugin() override;

    void loadEventsForDateRange(const QDate &startDate, const QDate &endDate) override;

public Q_SLOTS:
    void updateSettings();

private:
    void init();
    void emitDataChangedSignal(const QDate &startDate, const QDate &endDate);

    QDate m_lastStartDate;
    QDate m_lastEndDate;

    // Cache lookup data
    QCache<QDate, QCalendar::YearMonthDay> m_alternateDateCache;
    QCache<QDate, SubLabel> m_sublabelCache;

    // For updating config
    KConfigGroup m_generalConfigGroup;
    KConfigWatcher::Ptr m_configWatcher;

    CalendarSystem::System m_calendarSystem = CalendarSystem::Gregorian;
    int m_dateOffset = 0; // For the (tabular) Islamic Civil calendar
};

#endif
