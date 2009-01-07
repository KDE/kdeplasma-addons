/*
 * Copyright (C) 2008 Petri Damstén <damu@iki.fi>
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

#include "speed.h"
#include <KLocale>

Speed::Speed(QObject* parent)
: SimpleUnit(parent)
{
    setObjectName("speed");

    m_default = "m/s";

    m_units["ms"]                            = "m/s";
    m_units[i18n("meters per second")]       = "m/s";
    m_units["m/s"]                           = 1.0;
    m_units["kmh"]                           = "km/h";
    m_units[i18n("kilometers per hour")]     = "km/h";
    m_units["km/h"]                          = 1.0/3.6;

    m_units[i18n("miles per hour")]          = i18n("mph");
    m_units["mph"]                           = 0.44704;
	m_units[i18n("foot per second")]         = "fps";
	m_units[i18n("feet per second")]         = "fps";
	m_units[i18n("ft/s")]                    = "fps";
	m_units["fps"]                           = 0.3048;
	m_units[i18n("inch per second")]         = "ips";
	m_units[i18n("in/s")]                    = "ips";
	m_units["ips"]                           = 0.0254;

    m_units[i18n("knot")]                    = "kt";
    m_units[i18n("knots")]                   = "kt";
    m_units[i18n("nautical miles per hour")] = "kt";
    m_units["kt"]                            = 1.852/3.6;

    // http://en.wikipedia.org/wiki/Speed_of_sound
    m_units[i18n("speed of sound")]          = "Ma";
    m_units[i18n("machs")]                   = "Ma";
    m_units[i18n("mach")]                    = "Ma";
    m_units["Ma"]                            = 343.0; // In dry air at 20 °C

    m_units[i18n("speed of light")]          = "c";
    m_units["c"]                             = 299792458.0;
}

QString Speed::name() const
{
    return i18n("Speed");
}
