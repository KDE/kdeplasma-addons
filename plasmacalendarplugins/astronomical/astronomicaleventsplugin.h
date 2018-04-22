/*
    Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef ASTRONOMICALEVENTSPLUGIN_H
#define ASTRONOMICALEVENTSPLUGIN_H

// KF
#include <CalendarEvents/CalendarEventsPlugin>

class AstronomicalEventsPlugin : public CalendarEvents::CalendarEventsPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.CalendarEventsPlugin" FILE "astronomicaleventsplugin.json")
    Q_INTERFACES(CalendarEvents::CalendarEventsPlugin)

public:
    AstronomicalEventsPlugin();
    ~AstronomicalEventsPlugin() override;

    void loadEventsForDateRange(const QDate &startDate, const QDate &endDate) override;

private:
    bool m_lunarPhaseShown;
    bool m_seasonShown;
};

#endif
