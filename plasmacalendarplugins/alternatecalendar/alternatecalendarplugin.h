/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ALTERNATECALENDARPLUGIN_H
#define ALTERNATECALENDARPLUGIN_H

#include <memory>

#include <QCache>

#include <KConfigGroup>
#include <KConfigWatcher>

#include <CalendarEvents/CalendarEventsPlugin>

#include "calendarsystem.h"

class AbstractCalendarProvider;

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

    std::unique_ptr<AbstractCalendarProvider> m_calendarProvider;

    QDate m_lastStartDate;
    QDate m_lastEndDate;

    // Cache lookup data
    QCache<QDate, SubLabel> m_subLabelsCache;

    // For updating config
    KConfigGroup m_generalConfigGroup;
    KConfigWatcher::Ptr m_configWatcher;

    CalendarSystem::System m_calendarSystem = CalendarSystem::Gregorian;
    int m_dateOffset = 0; // For the (tabular) Islamic Civil calendar
};

#endif
