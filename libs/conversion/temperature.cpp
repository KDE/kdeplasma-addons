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

#include "temperature.h"
#include "conversioni18ncatalog.h"
#include <KDebug>
#include <KLocale>

class Celsius : public Conversion::Complex
{
    double toDefault(double value) const { return value + 273.15; };
    double fromDefault(double value) const { return value - 273.15; };
};

class Fahrenheit : public Conversion::Complex
{
    double toDefault(double value) const { return (value + 459.67) * 5.0 / 9.0; };
    double fromDefault(double value) const { return (value * 9.0 / 5.0) - 459.67; };
};

class Delisle : public Conversion::Complex
{
    double toDefault(double value) const { return 373.15 - (value * 2.0 / 3.0); };
    double fromDefault(double value) const { return (373.15 - value) * 3.0 / 2.0; };
};

class Newton : public Conversion::Complex
{
    double toDefault(double value) const { return (value * 100.0 / 33.0) + 273.15; };
    double fromDefault(double value) const { return (value - 273.15) * 33.0 / 100.0; };
};

class Reaumur : public Conversion::Complex
{
    double toDefault(double value) const { return (value * 5.0 / 4.0) + 273.15; };
    double fromDefault(double value) const { return (value - 273.15) * 4.0 / 5.0; };
};

class Romer : public Conversion::Complex
{
    double toDefault(double value) const { return (value - 7.5) * 40.0 / 21.0 + 273.15; };
    double fromDefault(double value) const { return (value - 273.15) * 21.0 / 40.0 + 7.5; };
};


Temperature::Temperature(QObject* parent)
: Conversion::UnitCategory(parent)
{
    Conversioni18nCatalog::loadCatalog();
    
    setObjectName("temperature");
    setName(i18n("Temperature"));

    setDefaultUnit(U(i18n("kelvin"), i18n("kelvins"), "K", 1.0, ));
    U(i18n("celsius"), i18n("celsiuses"), "\xb0""C", new Celsius(), << "C");
    U(i18n("fahrenheit"), i18n("fahrenheits"), "\xb0""F", new Fahrenheit(), << "F");
    U(i18n("rankine"), i18n("rankines"), "\xb0""R", 5.0 / 9.0, << "R");
    U(i18n("delisle"), i18n("delisles"), "\xb0""De", new Delisle(), << "De");
    U(i18n("newton"), i18n("newtons"), "\xb0""N", new Newton(), << "N");
    U(i18n("réaumur"), i18n("réaumurs"), "\xb0""R\xe9", new Reaumur(),
        << i18n("reaumur") << i18n("reaumurs") << "Re" << "R\xe9");
    U(i18n("rømer"), i18n("rømer"), "\xb0""R\xf8", new Romer(),
        << i18n("romer") << i18n("romers") << "Ro" << "R\xf8");
}
