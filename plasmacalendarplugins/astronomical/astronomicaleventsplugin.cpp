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

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "astronomicaleventsplugin.h"

// KF
#include <KSharedConfig>
#include <KConfigGroup>
#include <KHolidays/LunarPhase>
#include <KHolidays/AstroSeasons>
#include <KLocalizedString>
// Qt
#include <QDebug>

AstronomicalEventsPlugin::AstronomicalEventsPlugin()
    : CalendarEvents::CalendarEventsPlugin()
{
    auto config = KSharedConfig::openConfig(QStringLiteral("plasma_calendar_astronomicalevents"));
    const KConfigGroup generalConfig = config->group("General");

    m_lunarPhaseShown = generalConfig.readEntry("showLunarPhase", true);
    m_seasonShown = generalConfig.readEntry("showSeason", true);
}

AstronomicalEventsPlugin::~AstronomicalEventsPlugin()
{
}

void AstronomicalEventsPlugin::loadEventsForDateRange(const QDate &startDate, const QDate &endDate)
{
    QMultiHash<QDate, CalendarEvents::EventData> data;

    for (QDate date = startDate; date <= endDate; date = date.addDays(1)) {
        if (m_lunarPhaseShown) {
            const auto phase = KHolidays::LunarPhase::phaseAtDate(date);
            if (phase != KHolidays::LunarPhase::None) {
                CalendarEvents::EventData lunarPhaseData;
                lunarPhaseData.setIsAllDay(true);
                lunarPhaseData.setTitle(KHolidays::LunarPhase::phaseName(phase));
                lunarPhaseData.setEventType(CalendarEvents::EventData::Event);
                lunarPhaseData.setIsMinor(false);

                data.insert(date, lunarPhaseData);
            }
        }

        if (m_seasonShown) {
            const auto season = KHolidays::AstroSeasons::seasonAtDate(date);
            if (season != KHolidays::AstroSeasons::None) {
                CalendarEvents::EventData seasonData;
                seasonData.setIsAllDay(true);
                seasonData.setTitle(KHolidays::AstroSeasons::seasonName(season));
                seasonData.setEventType(CalendarEvents::EventData::Event);
                seasonData.setIsMinor(false);

                data.insert(date, seasonData);
            }
        }
    }

    Q_EMIT dataReady(data);
}
