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

#include "area.h"
#include "conversioni18ncatalog.h"
#include <KLocale>

Area::Area(QObject* parent)
: Conversion::UnitCategory(parent)
{
    Conversioni18nCatalog::loadCatalog();
    
    setObjectName("area");
    setName(i18n("Area"));
    addSIUnit("m\xb2", i18nc("area unit","meter"), i18nc("area unit","meters"), 2);
    addUnitMapValues(unit("hm\xb2"), QStringList() << i18nc("area unit", "hectares")
            << i18nc("area unit", "hectare"));

    U(i18nc("area unit", "acre"), i18nc("area unit", "acres"),
        i18nc("area unit", "acre"), 4046.8564224, );
    U(i18nc("area unit", "square foot"), i18nc("area unit", "square feet"),
        i18nc("area unit: square feet", "ft\xc2\xb2"), 0.09290304,
        <<  i18nc("area unit: square foot", "square ft")
        << i18nc("area unit: square foot", "sq foot") << i18nc("area unit: square foot", "sq ft")
        << i18nc("area unit: square feet", "sq feet") << i18nc("area unit: square feet", "feet\xc2\xb2"));
    U(i18nc("area unit", "square inch"), i18nc("area unit", "square inches"),
        i18nc("area unit: square inches", "in\xc2\xb2"), 0.00064516,
        << i18nc("area unit", "square inch") <<  i18nc("area unit: square inch", "square in")
        << i18nc("area unit: square inch", "sq inches") << i18nc("area unit: square inch", "sq inch")
        << i18nc("area unit: square inch", "sq in") <<  i18nc("area unit: square inches", "inch\xc2\xb2"));
    U(i18nc("area unit", "square mile"), i18nc("area unit", "square miles"),
        i18nc("area unit: square miles", "mi\xc2\xb2"), 2589988.110336,
        << i18nc("area unit: square miles", "square mi")
        << i18nc("area unit: square miles", "sq miles") << i18nc("area unit: square miles", "sq mile")
        << i18nc("area unit: square miles", "sq mi") << i18nc("area unit: square miles", "mile\xc2\xb2"));
}
