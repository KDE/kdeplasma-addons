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
#include <KDebug>
#include <KLocale>

class CelsiusConv : public Conversion::Complex
{
    double toDefault(double value) const { return value + 273.15; };
    double fromDefault(double value) const { return value - 273.15; };
};

class FahrenheitConv : public Conversion::Complex
{
    double toDefault(double value) const { return (value + 459.67) * 5.0 / 9.0; };
    double fromDefault(double value) const { return (value * 9.0 / 5.0) - 459.67; };
};

class DelisleConv : public Conversion::Complex
{
    double toDefault(double value) const { return 373.15 - (value * 2.0 / 3.0); };
    double fromDefault(double value) const { return (373.15 - value) * 3.0 / 2.0; };
};

class NewtonConv : public Conversion::Complex
{
    double toDefault(double value) const { return (value * 100.0 / 33.0) + 273.15; };
    double fromDefault(double value) const { return (value - 273.15) * 33.0 / 100.0; };
};

class ReaumurConv : public Conversion::Complex
{
    double toDefault(double value) const { return (value * 5.0 / 4.0) + 273.15; };
    double fromDefault(double value) const { return (value - 273.15) * 4.0 / 5.0; };
};

class RomerConv : public Conversion::Complex
{
    double toDefault(double value) const { return (value - 7.5) * 40.0 / 21.0 + 273.15; };
    double fromDefault(double value) const { return (value - 273.15) * 21.0 / 40.0 + 7.5; };
};


Temperature::Temperature(QObject* parent)
: Conversion::UnitCategory(parent)
{
    setObjectName("temperature");
    setName(i18n("Temperature"));

    setDefaultUnit(U(Temperature::Kelvin, 1,
      i18nc("temperature unit symbol", "K"),
      i18nc("unit description in lists", "kelvins"),
      i18nc("unit synonyms for matching user input", "kelvin;kelvins;K"),
      ki18nc("amount in units (real)", "%1 kelvins"),
      ki18ncp("amount in units (integer)", "%1 kelvin", "%1 kelvins")
    ));
    U(Temperature::Celsius, new CelsiusConv(),
      i18nc("temperature unit symbol", "°C"),
      i18nc("unit description in lists", "celsiuses"),
      i18nc("unit synonyms for matching user input", "celsius;celsiuses;°C;C"),
      ki18nc("amount in units (real)", "%1 celsiuses"),
      ki18ncp("amount in units (integer)", "%1 celsius", "%1 celsiuses")
    );
    U(Temperature::Fahrenheit, new FahrenheitConv(),
      i18nc("temperature unit symbol", "°F"),
      i18nc("unit description in lists", "fahrenheits"),
      i18nc("unit synonyms for matching user input", "fahrenheit;fahrenheits;°F;F"),
      ki18nc("amount in units (real)", "%1 fahrenheits"),
      ki18ncp("amount in units (integer)", "%1 fahrenheit", "%1 fahrenheits")
    );
    U(Temperature::Rankine, 0.555556,
      i18nc("temperature unit symbol", "°R"),
      i18nc("unit description in lists", "rankines"),
      i18nc("unit synonyms for matching user input", "rankine;rankines;°R;R"),
      ki18nc("amount in units (real)", "%1 rankines"),
      ki18ncp("amount in units (integer)", "%1 rankine", "%1 rankines")
    );
    U(Temperature::Delisle, new DelisleConv(),
      i18nc("temperature unit symbol", "°De"),
      i18nc("unit description in lists", "delisles"),
      i18nc("unit synonyms for matching user input", "delisle;delisles;°De;De"),
      ki18nc("amount in units (real)", "%1 delisles"),
      ki18ncp("amount in units (integer)", "%1 delisle", "%1 delisles")
    );
    U(Temperature::Newton, new NewtonConv(),
      i18nc("temperature unit symbol", "°N"),
      i18nc("unit description in lists", "newtons"),
      i18nc("unit synonyms for matching user input", "newton;newtons;°N;N"),
      ki18nc("amount in units (real)", "%1 newtons"),
      ki18ncp("amount in units (integer)", "%1 newton", "%1 newtons")
    );
    U(Temperature::Reaumur, new ReaumurConv(),
      i18nc("temperature unit symbol", "°Ré"),
      i18nc("unit description in lists", "réaumurs"),
      i18nc("unit synonyms for matching user input", "réaumur;réaumurs;°Ré;reaumur;reaumurs;Re;Ré"),
      ki18nc("amount in units (real)", "%1 réaumurs"),
      ki18ncp("amount in units (integer)", "%1 réaumur", "%1 réaumurs")
    );
    U(Temperature::Romer, new RomerConv(),
      i18nc("temperature unit symbol", "°Rø"),
      i18nc("unit description in lists", "rømer"),
      i18nc("unit synonyms for matching user input", "rømer;rømer;°Rø;romer;romers;Ro;Rø"),
      ki18nc("amount in units (real)", "%1 rømer"),
      ki18ncp("amount in units (integer)", "%1 rømer", "%1 rømer")
    );
}
