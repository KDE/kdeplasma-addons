/*
 *   Copyright (C) 2007-2009 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "volume.h"
#include "conversioni18ncatalog.h"
#include <KLocale>

Volume::Volume(QObject* parent)
: Conversion::UnitCategory(parent)
{
    Conversioni18nCatalog::loadCatalog();
    
    setObjectName("volume");
    setName(i18n("Volume"));

    addSIUnit("m\xb3", i18nc("area unit","meter"), i18nc("area unit","meters"), 3);
    addSIUnit("l", i18n("liter"), i18n("liters"), 1, 1000.0);

    U(i18n("cubic foot"), i18n("cubic feet"), i18n("ft\xc2\xb3"), 0.028316846592,
        << i18n("cubic ft") << i18n("cu foot") << i18n("cu ft")
        << i18n("cu feet") << i18n("feet\xc2\xb3"));

    U(i18n("cubic inch"), i18n("cubic inches"), i18n("in\xc2\xb3"), 0.000016387064,
        << i18n("cubic inch") << i18n("cubic in") << i18n("cu inches")
        << i18n("cu inch") << i18n("cu in") << i18n("inch\xc2\xb3"));

    U(i18n("cubic mile"), i18n("cubic miles"), i18n("mi\xc2\xb3"), 4168181825.440579584,
        << i18n("cubic mile") << i18n("cubic mi") << i18n("cu miles") << i18n("cu mile")
        << i18n("cu mi") << i18n("mile\xc2\xb3"));

    U(i18n("fluid ounce"), i18n("fluid ounces"), i18n("fl.oz."), 0.00002957353,
        << i18n("oz.fl.") << i18n("oz. fl.") << i18n("fl. oz.")
        << i18n("fl oz") << i18n("fluid ounce"));

    U(i18n("cup"), i18n("cups"), i18n("cp"), 0.0002365882, );

    U(i18n("gallon (U.S. liquid)"), i18n("gallons (U.S. liquid)"), i18n("gal"), 0.003785412,
        << i18n("gallon") << i18n("gallons"));

    U(i18n("pint (imperial)"), i18n("pints (imperial)"), "pt", 0.00056826125,
        << i18n("pint") << i18n("pints") << "p");
}
