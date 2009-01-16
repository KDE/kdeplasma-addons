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

#include "volume.h"
#include <KDebug>
#include <KLocale>

Volume::Volume(QObject* parent)
: SimpleUnit(parent)
{
    setObjectName("volume");

    m_default = "m\xb3";

    m_units[i18n("cubic meter")]          = "m\xb3";
    m_units[i18n("cubic meters")]         = "m\xb3";
    m_units["m\xb3"]                      = 1.0;
    m_units[i18n("cubic kilometer")]      = "km\xb3";
    m_units[i18n("cubic kilometers")]     = "km\xb3";
    m_units["km\xb3"]                     = 1E+9;
    m_units[i18n("cubic centimeter")]     = "cm\xb3";
    m_units[i18n("cubic centimeters")]    = "cm\xb3";
    m_units["cm\xb3"]                     = 1E-6;
    m_units[i18n("cubic millimeter")]     = "mm\xb3";
    m_units[i18n("cubic millimeters")]    = "mm\xb3";
    m_units["mm\xb3"]                     = 1E-9;

    m_units["Ym\xb3"]                     = 1E+72;
    m_units["Zm\xb3"]                     = 1E+63;
    m_units["Em\xb3"]                     = 1E+54;
    m_units["Pm\xb3"]                     = 1E+45;
    m_units["Tm\xb3"]                     = 1E+36;
    m_units["Gm\xb3"]                     = 1E+27;
    m_units["Mm\xb3"]                     = 1E+18;
    m_units["hm\xb3"]                     = 1E+6;
    m_units["dam\xb3"]                    = 1E+3;
    m_units["dm\xb3"]                     = 1E-3;
    m_units["µm\xb3"]                     = 1E-18;
    m_units["nm\xb3"]                     = 1E-27;
    m_units["pm\xb3"]                     = 1E-36;
    m_units["fm\xb3"]                     = 1E-45;
    m_units["am\xb3"]                     = 1E-54;
    m_units["zm\xb3"]                     = 1E-63;
    m_units["ym\xb3"]                     = 1E-72;

    m_units[i18n("liter")]                = "l";
    m_units[i18n("liters")]               = "l";
	m_units["L"]                          = "l";
    m_units[i18n("centiliter")]           = "cl";
    m_units[i18n("centiliters")]          = "cl";
    m_units[i18n("milliliter")]           = "ml";
    m_units[i18n("milliliters")]          = "ml";
    m_units["Yl"]                         = 1E+21;
    m_units["Zl"]                         = 1E+18;
    m_units["El"]                         = 1E+15;
    m_units["Pl"]                         = 1E+12;
    m_units["Tl"]                         = 1E+9;
    m_units["Gl"]                         = 1E+6;
    m_units["Ml"]                         = 1E+3;
    m_units["kl"]                         = 1.0;
    m_units["hl"]                         = 1E-1;
    m_units["dal"]                        = 1E-2;
    m_units["l"]                          = 1E-3;
    m_units["dl"]                         = 1E-4;
    m_units["cl"]                         = 1E-5;
    m_units["ml"]                         = 1E-6;
    m_units["µl"]                         = 1E-9;
    m_units["nl"]                         = 1E-12;
    m_units["pl"]                         = 1E-15;
    m_units["fl"]                         = 1E-18;
    m_units["al"]                         = 1E-21;
    m_units["zl"]                         = 1E-24;
    m_units["yl"]                         = 1E-27;

    m_units[i18n("cubic foot")]           = i18n("cubic feet");
    m_units[i18n("cubic ft")]             = i18n("cubic feet");
    m_units[i18n("cu foot")]              = i18n("cubic feet");
    m_units[i18n("cu ft")]                = i18n("cubic feet");
    m_units[i18n("cu feet")]              = i18n("cubic feet");
    m_units[i18n("feet\xc2\xb3")]         = i18n("cubic feet");
    m_units[i18n("ft\xc2\xb3")]           = i18n("cubic feet");
    m_units[i18n("cubic feet")]           = 0.028316846592;
    m_units[i18n("cubic inch")]           = i18n("cubic inches");
    m_units[i18n("cubic in")]             = i18n("cubic inches");
    m_units[i18n("cu inches")]            = i18n("cubic inches");
    m_units[i18n("cu inch")]              = i18n("cubic inches");
    m_units[i18n("cu in")]                = i18n("cubic inches");
    m_units[i18n("inch\xc2\xb3")]         = i18n("cubic inches");
    m_units[i18n("in\xc2\xb3")]           = i18n("cubic inches");
    m_units[i18n("cubic inches")]         = 0.000016387064;
    m_units[i18n("cubic mile")]           = i18n("cubic mile");
    m_units[i18n("cubic mi")]             = i18n("cubic mile");
    m_units[i18n("cu miles")]             = i18n("cubic mile");
    m_units[i18n("cu mile")]              = i18n("cubic mile");
    m_units[i18n("cu mi")]                = i18n("cubic mile");
    m_units[i18n("mile\xc2\xb3")]         = i18n("cubic mile");
    m_units[i18n("mi\xc2\xb3")]           = i18n("cubic mile");
    m_units[i18n("cubic miles")]          = 4168181825.440579584;

    m_units[i18n("oz.fl.")]               = i18n("fluid ounces");
    m_units[i18n("oz. fl.")]              = i18n("fluid ounces");
    m_units[i18n("fl.oz.")]               = i18n("fluid ounces");
    m_units[i18n("fl. oz.")]              = i18n("fluid ounces");
    m_units[i18n("fl oz")]                = i18n("fluid ounces");
    m_units[i18n("fluid ounce")]          = i18n("fluid ounces");
    m_units[i18n("fluid ounces")]         = 0.00002957353;
    m_units[i18n("cp")]                   = i18n("cups");
    m_units[i18n("cup")]                  = i18n("cups");
    m_units[i18n("cups")]                 = 0.0002365882;
    m_units[i18n("gal")]                  = i18n("gallons (U.S. liquid)");
    m_units[i18n("gallon")]               = i18n("gallons (U.S. liquid)");
    m_units[i18n("gallons")]              = i18n("gallons (U.S. liquid)");
    m_units[i18n("gallons (U.S. liquid)")] = 0.003785412;

    m_units[i18n("pint")]                 = i18n("pints (imperial)");
    m_units[i18n("pints")]                = i18n("pints (imperial)");
    m_units[i18n("pints (imperial)")]     = 0.00056826125;
}

QString Volume::name() const
{
    return i18n("Volume");
}

bool Volume::hasUnit(const QString &unit) const
{
    return SimpleUnit::hasUnit(replace(unit));
}

QString Volume::replace(const QString &unit) const
{
    QString result = unit;
    result.replace("/-3", "\xb3");
    result.replace("^3", "\xb3");
    result.replace('3', "\xb3");
    return result;
}

double Volume::toDefault(double value, const QString &unit) const
{
    return SimpleUnit::toDefault(value, replace(unit));
}

double Volume::fromDefault(double value, const QString &unit) const
{
    return SimpleUnit::fromDefault(value, replace(unit));
}
