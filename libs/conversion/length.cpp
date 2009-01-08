/*
 * Copyright (C) 2007 Petri Damstén <damu@iki.fi>
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

#include "length.h"
#include <KDebug>
#include <KLocale>

Length::Length(QObject* parent)
: SimpleUnit(parent)
{
    setObjectName("length");

    m_default = "m";

    addSIUnit(m_default, i18n("meter"), i18n("meters"));

    m_units[i18n("inch")]          = i18n("inches");
    m_units[i18n("in")]            = i18n("inches");
    m_units[i18n("\"")]            = i18n("inches");
    m_units[i18n("inches")]        = 0.0254;
    m_units[i18n("foot")]          = i18n("feet");
    m_units[i18n("ft")]            = i18n("feet");
    m_units[i18n("feet")]          = 0.3048;
    m_units[i18n("yard")]          = i18n("yards");
    m_units[i18n("yd")]            = i18n("yards");
    m_units[i18n("yards")]         = 0.9144;
    m_units[i18n("mile")]          = i18n("miles");
    m_units[i18n("ml")]            = i18n("miles");
    m_units[i18n("miles")]         = 1609.344;
    m_units[i18n("nautical mile")] = i18n("nautical miles");
    m_units[i18n("nmi")]           = i18n("nautical miles");
    m_units[i18n("nautical miles")] = 1852.0;

    m_units[i18n("ly")]            = "lightyears";
    m_units[i18n("light-year")]    = "lightyears";
    m_units[i18n("light-years")]   = "lightyears";
    m_units[i18n("lightyear")]     = "lightyears";
    m_units[i18n("lightyears")]    = 9460730472580800.0;
    m_units[i18n("pc")]            = "parsecs";
    m_units[i18n("parsec")]        = "parsecs";
    m_units[i18n("parsecs")]       = 3.085678E+16;
    m_units[i18n("au")]            = "astronomical units";
    m_units[i18n("astronomical unit")]  = "astronomical units";
    m_units[i18n("astronomical units")] = 149597870691.0;
}

QString Length::name() const
{
    return i18n("Length");
}
