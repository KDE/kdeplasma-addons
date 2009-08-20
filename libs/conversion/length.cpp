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

#include "length.h"
#include "conversioni18ncatalog.h"
#include <KDebug>
#include <KLocale>

Length::Length(QObject* parent)
: Conversion::UnitCategory(parent)
{
    Conversioni18nCatalog::loadCatalog();
    
    setObjectName("length");
    setName(i18n("Length"));

    addSIUnit("m", i18nc("length unit","meter"), i18nc("length unit","meters"));
    U(i18nc("length unit","inch"), i18nc("length unit","inches"),
        i18nc("length unit: inch","in"), 0.0254, << "\"");
    U(i18nc("length unit","foot"), i18nc("length unit","feet"),
        i18nc("length unit: foot","ft"), 0.3048, );
    U(i18nc("length unit","yard"), i18nc("length unit","yards"),
        i18nc("length unit: yard","yd"), 0.9144, );
    U(i18nc("length unit","mile"), i18nc("length unit","miles"),
        i18nc("length unit: mile","mi"), 1609.344, );
    U(i18nc("length unit","nautical mile"), i18nc("length unit","nautical miles"),
        i18nc("length unit: nautical mile","nmi"), 1852.0, );
    U(i18nc("length unit","light-year"), i18nc("length unit","light-years"),
        i18nc("length unit: light-year","ly"), 9460730472580800.0,
        << i18nc("length unit", "lightyear") << i18nc("length unit", "lightyears"));
    U(i18nc("length unit","parsec"), i18nc("length unit","parsecs"),
        i18nc("length unit: parsec","pc"), 3.085678E+16, );
    U(i18nc("length unit","astronomical unit"), i18nc("length unit","astronomical units"),
        i18nc("length unit: astronomical unit","au"), 149597870691.0, );
}

