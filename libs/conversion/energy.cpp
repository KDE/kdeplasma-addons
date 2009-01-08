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

#include "energy.h"
#include <KLocale>

Energy::Energy(QObject* parent)
: SimpleUnit(parent)
{
    setObjectName("energy");

    m_default = "J";

    addSIUnit(m_default, i18n("joule"), i18n("joules"));

    m_units[i18n("electronvolt")]  = "eV";
    m_units[i18n("electronvolts")] = "eV";
    m_units["eV"]                  = 1.60217653E-19;
    m_units[i18n("rydberg")]       = "Ry";
    m_units[i18n("rydbergs")]      = "Ry";
    m_units["Ry"]                  = 2.179872E-18;
    m_units[i18n("kilocalorie")]   = "kcal";
    m_units[i18n("kilocalories")]  = "kcal";
    m_units["kcal"]                = 4186.8;
}

QString Energy::name() const
{
    return i18n("Energy");
}
