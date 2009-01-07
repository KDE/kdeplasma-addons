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

    m_units[i18n("meter")]         = "m";
    m_units[i18n("meters")]        = "m";
    m_units["m"]                   = 1.0;
    m_units[i18n("kilometer")]     = "km";
    m_units[i18n("kilometers")]    = "km";
    m_units["km"]                  = 1000.0;
    m_units[i18n("decimeter")]     = "dm";
    m_units[i18n("decimeters")]    = "dm";
    m_units["dm"]                  = 0.1;
    m_units[i18n("centimeter")]    = "cm";
    m_units[i18n("centimeters")]   = "cm";
    m_units["cm"]                  = 0.01;
    m_units[i18n("millimeter")]    = "mm";
    m_units[i18n("millimeters")]   = "mm";
    m_units["mm"]                  = 0.001;
    m_units[i18n("micrometer")]    = "µm";
    m_units[i18n("micrometers")]   = "µm";
    m_units["µm"]                  = 0.000001;
    m_units[i18n("nanometer")]     = "nm";
    m_units[i18n("nanometers")]    = "nm";
    m_units["nm"]                  = 0.000000001;

    m_units["Ym"]                  = 1E+24;
    m_units["Zm"]                  = 1E+21;
    m_units["Em"]                  = 1E+18;
    m_units["Pm"]                  = 1E+15;
    m_units["Tm"]                  = 1E+12;
    m_units["Gm"]                  = 1E+9;
    m_units["Mm"]                  = 1E+6;
    m_units["hm"]                  = 1E+2;
    m_units["dam"]                 = 1E+1;

    m_units[i18n("picometer")]     = 1E-12;
    m_units[i18n("picometers")]    = 1E-12;
    m_units["pm"]                  = 1E-12;

    m_units[i18n("femtometer")]    = 1E-15;
    m_units[i18n("femtometer")]    = 1E-15;
    m_units["fm"]                  = 1E-15;
    m_units["am"]                  = 1E-18;
    m_units["zm"]                  = 1E-21;
    m_units["ym"]                  = 1E-24;

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
