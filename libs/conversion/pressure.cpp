/*
 * Copyright (C) 2009 Petri Damstén <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pressure.h"
#include <KLocale>

Pressure::Pressure(QObject* parent)
: SimpleUnit(parent)
{
    setObjectName("pressure");

    m_default = "Pa";

    addSIUnit(m_default, i18n("pascal"), i18n("pascals"));

    m_units["bar"]                = 100000.0;
    m_units[i18n("millibar")]     = "mbar";
    m_units[i18n("millibars")]    = "mbar";
    m_units["mb"]                 = "mbar";
    m_units["mbar"]               = 100.0;
    m_units[i18n("decibar")]      = "dbar";
    m_units[i18n("decibars")]     = "dbar";
    m_units["dbar"]               = 10000.0;
    m_units["Torr"]               = 133.322;
    m_units[i18n("technical atmosphere")] = "at";
    m_units["at"]                 = 98066.5;
    m_units[i18n("atmosphere")]   = "atm";
    m_units["atm"]                = 101325.0;
    m_units[i18n("pound-force per square inch")] = "psi";
    m_units["psi"]                = 6894.76;
    m_units[i18n("inches of mercury")] = "inHg";
    m_units["inHg"]               = 3386.389;
}

QString Pressure::name() const
{
    return i18n("Pressure");
}
