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

#include "temperature.h"
#include <KDebug>
#include <KLocale>

class Celsius : public Complex
{
    double toDefault(double value) const { return value + 273.15; };
    double fromDefault(double value) const { return value - 273.15; };
};

class Fahrenheit : public Complex
{
    double toDefault(double value) const { return (value + 459.67) * 5.0 / 9.0; };
    double fromDefault(double value) const { return (value * 9.0 / 5.0) - 459.67; };
};

class Delisle : public Complex
{
    double toDefault(double value) const { return 373.15 - (value * 2.0 / 3.0); };
    double fromDefault(double value) const { return (373.15 - value) * 3.0 / 2.0; };
};

class Newton : public Complex
{
    double toDefault(double value) const { return (value * 100.0 / 33.0) + 273.15; };
    double fromDefault(double value) const { return (value - 273.15) * 33.0 / 100.0; };
};

class Reaumur : public Complex
{
    double toDefault(double value) const { return (value * 5.0 / 4.0) + 273.15; };
    double fromDefault(double value) const { return (value - 273.15) * 4.0 / 5.0; };
};

class Romer : public Complex
{
    double toDefault(double value) const { return (value - 7.5) * 40.0 / 21.0 + 273.15; };
    double fromDefault(double value) const { return (value - 273.15) * 21.0 / 40.0 + 7.5; };
};


Temperature::Temperature(QObject* parent)
: SimpleUnit(parent)
{
    setObjectName("temperature");

    m_default = "K";

    m_units[i18n("kelvin")]       = "K";
    m_units[i18n("kelvins")]      = "K";
    m_units["K"]                  = 1.0;
    m_units[i18n("celsius")]      = "\xb0""C";
    m_units[i18n("celsiuses")]    = "\xb0""C";
    m_units["C"]                  = "\xb0""C";
    m_units["\xb0""C"]            = QVariant::fromValue(static_cast<Complex*>(new Celsius()));
    m_units[i18n("fahrenheit")]   = "\xb0""F";
    m_units[i18n("fahrenheits")]  = "\xb0""F";
    m_units["F"]                  = "\xb0""F";
    m_units["\xb0""F"]            = QVariant::fromValue(static_cast<Complex*>(new Fahrenheit()));
    m_units[i18n("rankine")]      = "\xb0""R";
    m_units[i18n("rankines")]     = "\xb0""R";
    m_units["R"]                  = "\xb0""R";
    m_units["\xb0""R"]            = 5.0 / 9.0;
    m_units[i18n("delisle")]      = "\xb0""De";
    m_units[i18n("delisles")]     = "\xb0""De";
    m_units["De"]                 = "\xb0""De";
    m_units["\xb0""De"]           = QVariant::fromValue(static_cast<Complex*>(new Delisle()));
    m_units[i18n("newton")]       = "\xb0""N";
    m_units[i18n("newtons")]      = "\xb0""N";
    m_units["N"]                  = "\xb0""N";
    m_units["\xb0""N"]            = QVariant::fromValue(static_cast<Complex*>(new Newton()));
    m_units[i18n("r\xe9""aumur")] = "\xb0""R\xe9";
    m_units[i18n("r\xe9""aumurs")]= "\xb0""R\xe9";
    m_units[i18n("reaumur")]      = "\xb0""R\xe9";
    m_units[i18n("reaumurs")]     = "\xb0""R\xe9";
    m_units["Re"]                 = "\xb0""R\xe9";
    m_units["R\xe9"]              = "\xb0""R\xe9";
    m_units["\xb0""R\xe9"]        = QVariant::fromValue(static_cast<Complex*>(new Reaumur()));
    m_units[i18n("r\xf8mer")]     = "\xb0""R\xf8";
    m_units[i18n("r\xf8mer")]     = "\xb0""R\xf8";
    m_units[i18n("romer")]        = "\xb0""R\xf8";
    m_units[i18n("romers")]       = "\xb0""R\xf8";
    m_units["Ro"]                 = "\xb0""R\xf8";
    m_units["R\xf8"]              = "\xb0""R\xf8";
    m_units["\xb0""R\xf8"]        = QVariant::fromValue(static_cast<Complex*>(new Romer()));
}

QString Temperature::name() const
{
    return i18n("Temperature");
}
