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

#include "fuel_efficiency.h"
#include "conversioni18ncatalog.h"
#include <KLocale>

class kmpl : public Conversion::Complex
{
    double toDefault(double value) const { return 100.0 / value; };
    double fromDefault(double value) const { return 100.0 / value; };
};

class mpg : public Conversion::Complex
{
    double toDefault(double value) const { return 235.2 / value; };
    double fromDefault(double value) const { return 235.2 / value; };
};

class mpgi : public Conversion::Complex
{
    double toDefault(double value) const { return 282.5 / value; };
    double fromDefault(double value) const { return 282.5 / value; };
};

FuelEfficiency::FuelEfficiency(QObject* parent)
: Conversion::UnitCategory(parent)
{
    Conversioni18nCatalog::loadCatalog();
	
	setObjectName("fuelefficiency");
    setName(i18n("Fuel Efficiency"));

    setDefaultUnit(U(i18n("liters per 100 kilometers"), i18n("liters per 100 kilometers"),
            "l/100 km", 1.0, << "L/100 km"));
    U(i18n("mile per US gallon"), i18n("miles per US gallon"),
            "mpg", new mpg(), );
    U(i18n("mile per imperial gallon"), i18n("miles per imperial gallon"),
            i18n("mpg (imperial)"), new mpgi(), );
    U(i18n("kilometre per litre"), i18n("kilometres per litre"), "kmpl", new kmpl(), << "km/l");
}
