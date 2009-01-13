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

    addSIUnit(m_default, i18nc("length unit","meter"), i18nc("length unit","meters"));

    m_units[i18nc("length unit","inch")]          			= i18n("inches");
    m_units[i18nc("length unit: inch","in")]            = i18n("inches");
    m_units[i18nc("length unit: inch","\"")]            = i18n("inches");
    m_units[i18nc("length unit","inches")]        	= 0.0254;
    m_units[i18nc("length unit","foot")]          	= i18n("feet");
    m_units[i18nc("length unit: foot","ft")]            = i18n("feet");
    m_units[i18nc("length unit","feet")]          	= 0.3048;
    m_units[i18nc("length unit","yard")]          	= i18n("yards");
    m_units[i18nc("length unit: yard","yd")]            = i18n("yards");
    m_units[i18nc("length unit","yards")]         	= 0.9144;
    m_units[i18nc("length unit","mile")]          	= i18n("miles");
    m_units[i18nc("length unit: mile","ml")]            = i18n("miles");
    m_units[i18nc("length unit","miles")]         	= 1609.344;
    m_units[i18nc("length unit","nautical mile")] 	= i18n("nautical miles");
    m_units[i18nc("length unit: nautical mile","nmi")]  = i18n("nautical miles");
    m_units[i18nc("length unit","nautical miles")] 	= 1852.0;

    m_units[i18nc("length unit: light-year","ly")]      = "lightyears";
    m_units[i18nc("length unit", "light-year")]    	= "lightyears";
    m_units[i18nc("length unit", "light-years")]   	= "lightyears";
    m_units[i18nc("length unit", "lightyear")]     	= "lightyears";
    m_units[i18nc("length unit", "lightyears")]    	= 9460730472580800.0;
    m_units[i18nc("length unit: parsec","pc")]          = "parsecs";
    m_units[i18nc("length unit", "parsec")]        	= "parsecs";
    m_units[i18nc("length unit", "parsecs")]       	= 3.085678E+16;
    m_units[i18nc("length unit: astronomical unit","au")] = "astronomical units";
    m_units[i18nc("length unit", "astronomical unit")]  = "astronomical units";
    m_units[i18nc("length unit", "astronomical units")] = 149597870691.0;
}

QString Length::name() const
{
    return i18n("Length");
}
