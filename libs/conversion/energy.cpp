/*
 *   Copyright (C) 2009 Petri Damstén <damu@iki.fi>
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

#include "energy.h"
#include "conversioni18ncatalog.h"
#include <KLocale>

Energy::Energy(QObject* parent)
: Conversion::UnitCategory(parent)
{
    Conversioni18nCatalog::loadCatalog();
	
	setObjectName("energy");
    setName(i18n("Energy"));
    addSIUnit("J", i18n("joule"), i18n("joules"));

    U(i18n("guideline daily amount"), i18n("guideline daily amount"), "GDA", 2000 * 4186.8, );
    U(i18n("electronvolt"), i18n("electronvolts"), "eV", 1.60217653E-19, );
    U(i18n("rydberg"), i18n("rydbergs"), "Ry", 2.179872E-18, );
    U(i18n("kilocalorie"), i18n("kilocalories"), "kcal", 4186.8, );
}
