/*
 * Copyright (C) 2007,2008 Petri Damstén <damu@iki.fi>
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

#include "area.h"
#include <KDebug>
#include <KLocale>

Area::Area(QObject* parent)
: SimpleUnit(parent)
{
    setObjectName("area");

    m_default = "m\xb2";

    m_units[i18n("square meter")]         = "m\xb2";
    m_units[i18n("square meters")]        = "m\xb2";
    m_units["m\xb2"]                      = 1.0;
    m_units[i18n("square kilometer")]     = "km\xb2";
    m_units[i18n("square kilometers")]    = "km\xb2";
    m_units["km\xb2"]                     = 1E+6;
    m_units[i18n("square centimeter")]    = "cm\xb2";
    m_units[i18n("square centimeters")]   = "cm\xb2";
    m_units["cm\xb2"]                     = 1E-4;
    m_units[i18n("square millimeter")]    = "mm\xb2";
    m_units[i18n("square millimeters")]   = "mm\xb2";
    m_units["mm\xb2"]                     = 1E-6;

    m_units[i18n("hectares")]             = "hm\xb2";
    m_units[i18n("hectare")]              = "hm\xb2";
    m_units[i18n("hectometer")]           = "hm\xb2";
    m_units[i18n("hectometers")]          = "hm\xb2";
    m_units["hm\xb2"]                     = 1E+4;

    m_units["Ym\xb2"]                     = 1E+48;
    m_units["Zm\xb2"]                     = 1E+42;
    m_units["Em\xb2"]                     = 1E+36;
    m_units["Pm\xb2"]                     = 1E+30;
    m_units["Tm\xb2"]                     = 1E+24;
    m_units["Gm\xb2"]                     = 1E+18;
    m_units["Mm\xb2"]                     = 1E+12;
    m_units["dam\xb2"]                    = 1E+2;
    m_units["dm\xb2"]                     = 1E-2;
    m_units["µm\xb2"]                     = 1E-12;
    m_units["nm\xb2"]                     = 1E-18;
    m_units["pm\xb2"]                     = 1E-24;
    m_units["fm\xb2"]                     = 1E-30;
    m_units["am\xb2"]                     = 1E-36;
    m_units["zm\xb2"]                     = 1E-42;
    m_units["ym\xb2"]                     = 1E-48;

    m_units[i18n("acre")]                 = 4046.9;
    m_units[i18n("square foot")]          = i18n("square feet");
    m_units[i18n("square ft")]            = i18n("square feet");
    m_units[i18n("sq foot")]              = i18n("square feet");
    m_units[i18n("sq ft")]                = i18n("square feet");
    m_units[i18n("sq feet")]              = i18n("square feet");
    m_units[i18n("feet\xc2\xb2")]         = i18n("square feet");
    m_units[i18n("ft\xc2\xb2")]           = i18n("square feet");
    m_units[i18n("square feet")]          = 0.09290304;
    m_units[i18n("square inch")]          = i18n("square inches");
    m_units[i18n("square in")]            = i18n("square inches");
    m_units[i18n("sq inches")]            = i18n("square inches");
    m_units[i18n("sq inch")]              = i18n("square inches");
    m_units[i18n("sq in")]                = i18n("square inches");
    m_units[i18n("inch\xc2\xb2")]         = i18n("square inches");
    m_units[i18n("in\xc2\xb2")]           = i18n("square inches");
    m_units[i18n("square inches")]        = 0.00064516;
    m_units[i18n("square mile")]          = i18n("square mile");
    m_units[i18n("square mi")]            = i18n("square mile");
    m_units[i18n("sq miles")]             = i18n("square mile");
    m_units[i18n("sq mile")]              = i18n("square mile");
    m_units[i18n("sq mi")]                = i18n("square mile");
    m_units[i18n("mile\xc2\xb2")]         = i18n("square mile");
    m_units[i18n("mi\xc2\xb2")]           = i18n("square mile");
    m_units[i18n("square miles")]         = 2589988.110336;
}

QString Area::name() const
{
    return i18n("Area");
}

bool Area::hasUnit(const QString &unit) const
{
    return SimpleUnit::hasUnit(replace(unit));
}

QString Area::replace(const QString &unit) const
{
    QString result = unit;
    result.replace("/-2", "\xb2");
    result.replace("^2", "\xb2");
    result.replace("2", "\xb2");
    return result;
}

double Area::toDefault(double value, const QString &unit) const
{
    return SimpleUnit::toDefault(value, replace(unit));
}

double Area::fromDefault(double value, const QString &unit) const
{
    return SimpleUnit::fromDefault(value, replace(unit));
}
