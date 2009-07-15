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

#include "density.h"
#include <KLocale>
#include <KDebug>

Density::Density(QObject* parent)
: Conversion::UnitCategory(parent)
{
    setObjectName("density");
    setName(i18n("Density"));

    U(Density::YottakilogramsPerCubicMeter, 1e+24,
      i18nc("density unit symbol", "YKg/m³"),
      i18nc("unit description in lists", "yottakilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "yottakilogram per cubic meter;yottakilograms per cubic meter;YKg/m³"),
      ki18nc("amount in units (real)", "%1 yottakilograms per cubic meter"),
      ki18ncp("amount in units (integer)",
              "%1 yottakilogram per cubic meter", "%1 yottakilograms per cubic meter")
    );
    U(Density::ZettakilogramPerCubicMeter, 1e+21,
      i18nc("density unit symbol", "ZKg/m³"),
      i18nc("unit description in lists", "zettakilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "zettakilogram per cubic meter;zettakilograms per cubic meter;ZKg/m³"),
      ki18nc("amount in units (real)", "%1 zettakilograms per cubic meter"),
      ki18ncp("amount in units (integer)", "%1 zettakilogram per cubic meter",
              "%1 zettakilograms per cubic meter")
    );
    U(Density::ExakilogramPerCubicMeter, 1e+18,
      i18nc("density unit symbol", "EKg/m³"),
      i18nc("unit description in lists", "exakilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "exakilogram per cubic meter;exakilograms per cubic meter;EKg/m³"),
      ki18nc("amount in units (real)", "%1 exakilograms per cubic meter"),
      ki18ncp("amount in units (integer)", "%1 exakilogram per cubic meter",
              "%1 exakilograms per cubic meter")
    );
    U(Density::PetakilogramPerCubicMeter, 1e+15,
      i18nc("density unit symbol", "PKg/m³"),
      i18nc("unit description in lists", "petakilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "petakilogram per cubic meter;petakilograms per cubic meter;PKg/m³"),
      ki18nc("amount in units (real)", "%1 petakilograms per cubic meter"),
      ki18ncp("amount in units (integer)", "%1 petakilogram per cubic meter",
              "%1 petakilograms per cubic meter")
    );
    U(Density::TerakilogramPerCubicMeter, 1e+12,
      i18nc("density unit symbol", "TKg/m³"),
      i18nc("unit description in lists", "terakilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "terakilogram per cubic meter;terakilograms per cubic meter;TKg/m³"),
      ki18nc("amount in units (real)", "%1 terakilograms per cubic meter"),
      ki18ncp("amount in units (integer)", "%1 terakilogram per cubic meter",
              "%1 terakilograms per cubic meter")
    );
    U(Density::GigakilogramPerCubicMeter, 1e+09,
      i18nc("density unit symbol", "GKg/m³"),
      i18nc("unit description in lists", "gigakilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "gigakilogram per cubic meter;gigakilograms per cubic meter;GKg/m³"),
      ki18nc("amount in units (real)", "%1 gigakilograms per cubic meter"),
      ki18ncp("amount in units (integer)", "%1 gigakilogram per cubic meter",
              "%1 gigakilograms per cubic meter")
    );
    U(Density::MegakilogramPerCubicMeter, 1e+06,
      i18nc("density unit symbol", "MKg/m³"),
      i18nc("unit description in lists", "megakilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "megakilogram per cubic meter;megakilograms per cubic meter;MKg/m³"),
      ki18nc("amount in units (real)", "%1 megakilograms per cubic meter"),
      ki18ncp("amount in units (integer)", "%1 megakilogram per cubic meter",
              "%1 megakilograms per cubic meter")
    );
    U(Density::KilokilogramPerCubicMeter, 1000,
      i18nc("density unit symbol", "kKg/m³"),
      i18nc("unit description in lists", "kilokilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "kilokilogram per cubic meter;kilokilograms per cubic meter;kKg/m³"),
      ki18nc("amount in units (real)", "%1 kilokilograms per cubic meter"),
      ki18ncp("amount in units (integer)", "%1 kilokilogram per cubic meter",
              "%1 kilokilograms per cubic meter")
    );
    U(Density::HectokilogramsPerCubicMeter, 100,
      i18nc("density unit symbol", "hKg/m³"),
      i18nc("unit description in lists", "hectokilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "hectokilogram per cubic meter;hectokilograms per cubic meter;hKg/m³"),
      ki18nc("amount in units (real)", "%1 hectokilograms per cubic meter"),
      ki18ncp("amount in units (integer)",
              "%1 hectokilogram per cubic meter", "%1 hectokilograms per cubic meter")
    );
    U(Density::DecakilogramsPerCubicMeter, 10,
      i18nc("density unit symbol", "daKg/m³"),
      i18nc("unit description in lists", "decakilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "decakilogram per cubic meter;decakilograms per cubic meter;daKg/m³"),
      ki18nc("amount in units (real)", "%1 decakilograms per cubic meter"),
      ki18ncp("amount in units (integer)",
              "%1 decakilogram per cubic meter", "%1 decakilograms per cubic meter")
    );
    setDefaultUnit(U(Density::KilogramsPerCubicMeter, 1,
      i18nc("density unit symbol", "Kg/m³"),
      i18nc("unit description in lists", "kilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "kilogram per cubic meter;kilograms per cubic meter;Kg/m³"),
      ki18nc("amount in units (real)", "%1 kilograms per cubic meter"),
      ki18ncp("amount in units (integer)",
              "%1 kilogram per cubic meter", "%1 kilograms per cubic meter")
    ));
    U(Density::DecikilogramsPerCubicMeter, 0.1,
      i18nc("density unit symbol", "dKg/m³"),
      i18nc("unit description in lists", "decikilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "decikilogram per cubic meter;decikilograms per cubic meter;dKg/m³"),
      ki18nc("amount in units (real)", "%1 decikilograms per cubic meter"),
      ki18ncp("amount in units (integer)",
              "%1 decikilogram per cubic meter", "%1 decikilograms per cubic meter")
    );
    U(Density::CentikilogramsPerCubicMeter, 0.01,
      i18nc("density unit symbol", "cKg/m³"),
      i18nc("unit description in lists", "centikilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "centikilogram per cubic meter;centikilograms per cubic meter;cKg/m³"),
      ki18nc("amount in units (real)", "%1 centikilograms per cubic meter"),
      ki18ncp("amount in units (integer)",
              "%1 centikilogram per cubic meter", "%1 centikilograms per cubic meter")
    );
    U(Density::MillikilogramsPerCubicMeter, 0.001,
      i18nc("density unit symbol", "mKg/m³"),
      i18nc("unit description in lists", "millikilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "millikilogram per cubic meter;millikilograms per cubic meter;mKg/m³"),
      ki18nc("amount in units (real)", "%1 millikilograms per cubic meter"),
      ki18ncp("amount in units (integer)",
              "%1 millikilogram per cubic meter", "%1 millikilograms per cubic meter")
    );
    U(Density::MicrokilogramsPerCubicMeter, 1e-06,
      i18nc("density unit symbol", "µKg/m³"),
      i18nc("unit description in lists", "microkilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "microkilogram per cubic meter;microkilograms per cubic meter;µKg/m³;uKg/m³"),
      ki18nc("amount in units (real)", "%1 microkilograms per cubic meter"),
      ki18ncp("amount in units (integer)",
              "%1 microkilogram per cubic meter", "%1 microkilograms per cubic meter")
    );
    U(Density::NanokilogramsPerCubicMeter, 1e-09,
      i18nc("density unit symbol", "nKg/m³"),
      i18nc("unit description in lists", "nanokilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "nanokilogram per cubic meter;nanokilograms per cubic meter;nKg/m³"),
      ki18nc("amount in units (real)", "%1 nanokilograms per cubic meter"),
      ki18ncp("amount in units (integer)",
              "%1 nanokilogram per cubic meter", "%1 nanokilograms per cubic meter")
    );
    U(Density::PicokilogramsPerCubicMeter, 1e-12,
      i18nc("density unit symbol", "pKg/m³"),
      i18nc("unit description in lists", "picokilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "picokilogram per cubic meter;picokilograms per cubic meter;pKg/m³"),
      ki18nc("amount in units (real)", "%1 picokilograms per cubic meter"),
      ki18ncp("amount in units (integer)",
              "%1 picokilogram per cubic meter", "%1 picokilograms per cubic meter")
    );
    U(Density::FemtokilogramsPerCubicMeter, 1e-15,
      i18nc("density unit symbol", "fKg/m³"),
      i18nc("unit description in lists", "femtokilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "femtokilogram per cubic meter;femtokilograms per cubic meter;fKg/m³"),
      ki18nc("amount in units (real)", "%1 femtokilograms per cubic meter"),
      ki18ncp("amount in units (integer)",
              "%1 femtokilogram per cubic meter", "%1 femtokilograms per cubic meter")
    );
    U(Density::AttokilogramsPerCubicMeter, 1e-18,
      i18nc("density unit symbol", "aKg/m³"),
      i18nc("unit description in lists", "attokilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "attokilogram per cubic meter;attokilograms per cubic meter;aKg/m³"),
      ki18nc("amount in units (real)", "%1 attokilograms per cubic meter"),
      ki18ncp("amount in units (integer)",
              "%1 attokilogram per cubic meter", "%1 attokilograms per cubic meter")
    );
    U(Density::ZeptokilogramsPerCubicMeter, 1e-21,
      i18nc("density unit symbol", "zKg/m³"),
      i18nc("unit description in lists", "zeptokilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "zeptokilogram per cubic meter;zeptokilograms per cubic meter;zKg/m³"),
      ki18nc("amount in units (real)", "%1 zeptokilograms per cubic meter"),
      ki18ncp("amount in units (integer)",
              "%1 zeptokilogram per cubic meter", "%1 zeptokilograms per cubic meter")
    );
    U(Density::YoctokilogramsPerCubicMeter, 1e-24,
      i18nc("density unit symbol", "yKg/m³"),
      i18nc("unit description in lists", "yoctokilograms per cubic meter"),
      i18nc("unit synonyms for matching user input",
            "yoctokilogram per cubic meter;yoctokilograms per cubic meter;yKg/m³"),
      ki18nc("amount in units (real)", "%1 yoctokilograms per cubic meter"),
      ki18ncp("amount in units (integer)",
              "%1 yoctokilogram per cubic meter", "%1 yoctokilograms per cubic meter")
    );
    // source http://www.coleparmer.com/techinfo/converters/engndensity.asp
    U(Density::KilogramPerLiter, 1000,
      i18nc("density unit symbol", "kg/l"),
      i18nc("unit description in lists", "kilograms per liter"),
      i18nc("unit synonyms for matching user input",
            "kilogram per liter;kilograms per liter;kg/l"),
      ki18nc("amount in units (real)", "%1 kilograms per liter"),
      ki18ncp("amount in units (integer)", "%1 kilogram per liter", "%1 kilograms per liter")
    );
    U(Density::GramPerLiter, 1,
      i18nc("density unit symbol", "g/l"),
      i18nc("unit description in lists", "grams per liter"),
      i18nc("unit synonyms for matching user input", "gram per liter;grams per liter;g/l"),
      ki18nc("amount in units (real)", "%1 grams per liter"),
      ki18ncp("amount in units (integer)", "%1 gram per liter", "%1 grams per liter")
    );
    U(Density::GramPerMilliliter, 1000,
      i18nc("density unit symbol", "g/ml"),
      i18nc("unit description in lists", "grams per milliliter"),
      i18nc("unit synonyms for matching user input",
            "gram per milliliter;grams per milliliter;g/ml"),
      ki18nc("amount in units (real)", "%1 grams per milliliter"),
      ki18ncp("amount in units (integer)", "%1 gram per milliliter", "%1 grams per milliliter")
    );
    U(Density::OuncePerCubicInch, 1729.99,
      i18nc("density unit symbol", "oz/in³"),
      i18nc("unit description in lists", "ounces per cubic inch"),
      i18nc("unit synonyms for matching user input",
            "ounce per cubic inch;ounces per cubic inch;oz/in³"),
      ki18nc("amount in units (real)", "%1 ounces per cubic inch"),
      ki18ncp("amount in units (integer)", "%1 ounce per cubic inch", "%1 ounces per cubic inch")
    );
    U(Density::OuncePerCubicYard, 1.00115,
      i18nc("density unit symbol", "lb/yd³"),
      i18nc("unit description in lists", "ounces per cubic yard"),
      i18nc("unit synonyms for matching user input",
            "ounce per cubic foot;ounces per cubic yard;lb/yd³"),
      ki18nc("amount in units (real)", "%1 ounces per cubic yard"),
      ki18ncp("amount in units (integer)", "%1 ounce per cubic foot", "%1 ounces per cubic yard")
    );
    U(Density::PoundPerCubicInch, 27679.9,
      i18nc("density unit symbol", "lb/in³"),
      i18nc("unit description in lists", "pounds per cubic inch"),
      i18nc("unit synonyms for matching user input",
            "pound per cubic inch;pounds per cubic inch;lb/in³"),
      ki18nc("amount in units (real)", "%1 pounds per cubic inch"),
      ki18ncp("amount in units (integer)", "%1 pound per cubic inch", "%1 pounds per cubic inch")
    );
    U(Density::PoundPerCubicFoot, 16.0185,
      i18nc("density unit symbol", "lb/ft³"),
      i18nc("unit description in lists", "pounds per cubic foot"),
      i18nc("unit synonyms for matching user input",
            "pound per cubic foot;pounds per cubic foot;lb/ft³"),
      ki18nc("amount in units (real)", "%1 pounds per cubic foot"),
      ki18ncp("amount in units (integer)", "%1 pound per cubic foot", "%1 pounds per cubic foot")
    );
    U(Density::PoundPerCubicYard, 0.593276,
      i18nc("density unit symbol", "lb/yd³"),
      i18nc("unit description in lists", "pounds per cubic yard"),
      i18nc("unit synonyms for matching user input",
            "pound per cubic yard;pounds per cubic yard;lb/yd³"),
      ki18nc("amount in units (real)", "%1 pounds per cubic yard"),
      ki18ncp("amount in units (integer)", "%1 pound per cubic yard", "%1 pounds per cubic yard")
    );
}
