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

#include "configstorage.h"

// KF
#include <KSharedConfig>


ConfigStorage::ConfigStorage(QObject *parent)
    : QObject(parent)
{
    auto config = KSharedConfig::openConfig(QStringLiteral("plasma_calendar_astronomicalevents"));
    m_generalConfigGroup = config->group("General");

    m_lunarPhaseShown = m_generalConfigGroup.readEntry("showLunarPhase", true);
    m_seasonShown = m_generalConfigGroup.readEntry("showSeason", true);
}

void ConfigStorage::save()
{
    m_generalConfigGroup.writeEntry("showLunarPhase", m_lunarPhaseShown);
    m_generalConfigGroup.writeEntry("showSeason", m_seasonShown);

    m_generalConfigGroup.sync();
}

