/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ALTERNATECALENDARPLUGIN_H
#define ALTERNATECALENDARPLUGIN_H

#include <memory>

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

private:
    const std::unique_ptr<class AlternateCalendarPluginPrivate> d;
};

#endif
