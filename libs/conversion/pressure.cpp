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

#include "pressure.h"
#include <KLocale>

Pressure::Pressure(QObject* parent)
: Conversion::UnitCategory(parent)
{
    setObjectName("pressure");
    setName(i18n("Pressure"));

    U(Pressure::Yottapascal, 1e+24,
      i18nc("pressure unit symbol", "YPa"),
      i18nc("unit description in lists", "yottapascals"),
      i18nc("unit synonyms for matching user input", "yottapascal;yottapascals;YPa"),
      ki18nc("amount in units (real)", "%1 yottapascals"),
      ki18ncp("amount in units (integer)", "%1 yottapascal", "%1 yottapascals")
    );
    U(Pressure::Zettapascal, 1e+21,
      i18nc("pressure unit symbol", "ZPa"),
      i18nc("unit description in lists", "zettapascals"),
      i18nc("unit synonyms for matching user input", "zettapascal;zettapascals;ZPa"),
      ki18nc("amount in units (real)", "%1 zettapascals"),
      ki18ncp("amount in units (integer)", "%1 zettapascal", "%1 zettapascals")
    );
    U(Pressure::Exapascal, 1e+18,
      i18nc("pressure unit symbol", "EPa"),
      i18nc("unit description in lists", "exapascals"),
      i18nc("unit synonyms for matching user input", "exapascal;exapascals;EPa"),
      ki18nc("amount in units (real)", "%1 exapascals"),
      ki18ncp("amount in units (integer)", "%1 exapascal", "%1 exapascals")
    );
    U(Pressure::Petapascal, 1e+15,
      i18nc("pressure unit symbol", "PPa"),
      i18nc("unit description in lists", "petapascals"),
      i18nc("unit synonyms for matching user input", "petapascal;petapascals;PPa"),
      ki18nc("amount in units (real)", "%1 petapascals"),
      ki18ncp("amount in units (integer)", "%1 petapascal", "%1 petapascals")
    );
    U(Pressure::Terapascal, 1e+12,
      i18nc("pressure unit symbol", "TPa"),
      i18nc("unit description in lists", "terapascals"),
      i18nc("unit synonyms for matching user input", "terapascal;terapascals;TPa"),
      ki18nc("amount in units (real)", "%1 terapascals"),
      ki18ncp("amount in units (integer)", "%1 terapascal", "%1 terapascals")
    );
    U(Pressure::Gigapascal, 1e+09,
      i18nc("pressure unit symbol", "GPa"),
      i18nc("unit description in lists", "gigapascals"),
      i18nc("unit synonyms for matching user input", "gigapascal;gigapascals;GPa"),
      ki18nc("amount in units (real)", "%1 gigapascals"),
      ki18ncp("amount in units (integer)", "%1 gigapascal", "%1 gigapascals")
    );
    U(Pressure::Megapascal, 1e+06,
      i18nc("pressure unit symbol", "MPa"),
      i18nc("unit description in lists", "megapascals"),
      i18nc("unit synonyms for matching user input", "megapascal;megapascals;MPa"),
      ki18nc("amount in units (real)", "%1 megapascals"),
      ki18ncp("amount in units (integer)", "%1 megapascal", "%1 megapascals")
    );
    U(Pressure::Kilopascal, 1000,
      i18nc("pressure unit symbol", "kPa"),
      i18nc("unit description in lists", "kilopascals"),
      i18nc("unit synonyms for matching user input", "kilopascal;kilopascals;kPa"),
      ki18nc("amount in units (real)", "%1 kilopascals"),
      ki18ncp("amount in units (integer)", "%1 kilopascal", "%1 kilopascals")
    );
    U(Pressure::Hectopascal, 100,
      i18nc("pressure unit symbol", "hPa"),
      i18nc("unit description in lists", "hectopascals"),
      i18nc("unit synonyms for matching user input", "hectopascal;hectopascals;hPa"),
      ki18nc("amount in units (real)", "%1 hectopascals"),
      ki18ncp("amount in units (integer)", "%1 hectopascal", "%1 hectopascals")
    );
    U(Pressure::Decapascal, 10,
      i18nc("pressure unit symbol", "daPa"),
      i18nc("unit description in lists", "decapascals"),
      i18nc("unit synonyms for matching user input", "decapascal;decapascals;daPa"),
      ki18nc("amount in units (real)", "%1 decapascals"),
      ki18ncp("amount in units (integer)", "%1 decapascal", "%1 decapascals")
    );
    setDefaultUnit(U(Pressure::Pascal, 1,
      i18nc("pressure unit symbol", "Pa"),
      i18nc("unit description in lists", "pascals"),
      i18nc("unit synonyms for matching user input", "pascal;pascals;Pa"),
      ki18nc("amount in units (real)", "%1 pascals"),
      ki18ncp("amount in units (integer)", "%1 pascal", "%1 pascals")
    ));
    U(Pressure::Decipascal, 0.1,
      i18nc("pressure unit symbol", "dPa"),
      i18nc("unit description in lists", "decipascals"),
      i18nc("unit synonyms for matching user input", "decipascal;decipascals;dPa"),
      ki18nc("amount in units (real)", "%1 decipascals"),
      ki18ncp("amount in units (integer)", "%1 decipascal", "%1 decipascals")
    );
    U(Pressure::Centipascal, 0.01,
      i18nc("pressure unit symbol", "cPa"),
      i18nc("unit description in lists", "centipascals"),
      i18nc("unit synonyms for matching user input", "centipascal;centipascals;cPa"),
      ki18nc("amount in units (real)", "%1 centipascals"),
      ki18ncp("amount in units (integer)", "%1 centipascal", "%1 centipascals")
    );
    U(Pressure::Millipascal, 0.001,
      i18nc("pressure unit symbol", "mPa"),
      i18nc("unit description in lists", "millipascals"),
      i18nc("unit synonyms for matching user input", "millipascal;millipascals;mPa"),
      ki18nc("amount in units (real)", "%1 millipascals"),
      ki18ncp("amount in units (integer)", "%1 millipascal", "%1 millipascals")
    );
    U(Pressure::Micropascal, 1e-06,
      i18nc("pressure unit symbol", "µPa"),
      i18nc("unit description in lists", "micropascals"),
      i18nc("unit synonyms for matching user input", "micropascal;micropascals;µPa;uPa"),
      ki18nc("amount in units (real)", "%1 micropascals"),
      ki18ncp("amount in units (integer)", "%1 micropascal", "%1 micropascals")
    );
    U(Pressure::Nanopascal, 1e-09,
      i18nc("pressure unit symbol", "nPa"),
      i18nc("unit description in lists", "nanopascals"),
      i18nc("unit synonyms for matching user input", "nanopascal;nanopascals;nPa"),
      ki18nc("amount in units (real)", "%1 nanopascals"),
      ki18ncp("amount in units (integer)", "%1 nanopascal", "%1 nanopascals")
    );
    U(Pressure::Picopascal, 1e-12,
      i18nc("pressure unit symbol", "pPa"),
      i18nc("unit description in lists", "picopascals"),
      i18nc("unit synonyms for matching user input", "picopascal;picopascals;pPa"),
      ki18nc("amount in units (real)", "%1 picopascals"),
      ki18ncp("amount in units (integer)", "%1 picopascal", "%1 picopascals")
    );
    U(Pressure::Femtopascal, 1e-15,
      i18nc("pressure unit symbol", "fPa"),
      i18nc("unit description in lists", "femtopascals"),
      i18nc("unit synonyms for matching user input", "femtopascal;femtopascals;fPa"),
      ki18nc("amount in units (real)", "%1 femtopascals"),
      ki18ncp("amount in units (integer)", "%1 femtopascal", "%1 femtopascals")
    );
    U(Pressure::Attopascal, 1e-18,
      i18nc("pressure unit symbol", "aPa"),
      i18nc("unit description in lists", "attopascals"),
      i18nc("unit synonyms for matching user input", "attopascal;attopascals;aPa"),
      ki18nc("amount in units (real)", "%1 attopascals"),
      ki18ncp("amount in units (integer)", "%1 attopascal", "%1 attopascals")
    );
    U(Pressure::Zeptopascal, 1e-21,
      i18nc("pressure unit symbol", "zPa"),
      i18nc("unit description in lists", "zeptopascals"),
      i18nc("unit synonyms for matching user input", "zeptopascal;zeptopascals;zPa"),
      ki18nc("amount in units (real)", "%1 zeptopascals"),
      ki18ncp("amount in units (integer)", "%1 zeptopascal", "%1 zeptopascals")
    );
    U(Pressure::Yoctopascal, 1e-24,
      i18nc("pressure unit symbol", "yPa"),
      i18nc("unit description in lists", "yoctopascals"),
      i18nc("unit synonyms for matching user input", "yoctopascal;yoctopascals;yPa"),
      ki18nc("amount in units (real)", "%1 yoctopascals"),
      ki18ncp("amount in units (integer)", "%1 yoctopascal", "%1 yoctopascals")
    );
    U(Pressure::Bar, 100000,
      i18nc("pressure unit symbol", "bar"),
      i18nc("unit description in lists", "bars"),
      i18nc("unit synonyms for matching user input", "bar;bars;bar"),
      ki18nc("amount in units (real)", "%1 bars"),
      ki18ncp("amount in units (integer)", "%1 bar", "%1 bars")
    );
    U(Pressure::Millibar, 100,
      i18nc("pressure unit symbol", "mbar"),
      i18nc("unit description in lists", "millibars"),
      i18nc("unit synonyms for matching user input", "millibar;millibars;mbar;mb"),
      ki18nc("amount in units (real)", "%1 millibars"),
      ki18ncp("amount in units (integer)", "%1 millibar", "%1 millibars")
    );
    U(Pressure::Decibar, 10000,
      i18nc("pressure unit symbol", "dbar"),
      i18nc("unit description in lists", "decibars"),
      i18nc("unit synonyms for matching user input", "decibar;decibars;dbar"),
      ki18nc("amount in units (real)", "%1 decibars"),
      ki18ncp("amount in units (integer)", "%1 decibar", "%1 decibars")
    );
    U(Pressure::Torr, 133.322,
      i18nc("pressure unit symbol", "torr"),
      i18nc("unit description in lists", "torrs"),
      i18nc("unit synonyms for matching user input", "torr;torrs;torr"),
      ki18nc("amount in units (real)", "%1 torrs"),
      ki18ncp("amount in units (integer)", "%1 torr", "%1 torrs")
    );
    U(Pressure::TechnicalAtmosphere, 98066.5,
      i18nc("pressure unit symbol", "at"),
      i18nc("unit description in lists", "technical atmospheres"),
      i18nc("unit synonyms for matching user input",
            "technical atmosphere;technical atmospheres;at"),
      ki18nc("amount in units (real)", "%1 technical atmospheres"),
      ki18ncp("amount in units (integer)", "%1 technical atmosphere", "%1 technical atmospheres")
    );
    U(Pressure::Atmosphere, 101325,
      i18nc("pressure unit symbol", "atm"),
      i18nc("unit description in lists", "atmospheres"),
      i18nc("unit synonyms for matching user input", "atmosphere;atmospheres;atm"),
      ki18nc("amount in units (real)", "%1 atmospheres"),
      ki18ncp("amount in units (integer)", "%1 atmosphere", "%1 atmospheres")
    );
    U(Pressure::PoundForcePerSquareInch, 6894.76,
      i18nc("pressure unit symbol", "psi"),
      i18nc("unit description in lists", "pound-force per square inch"),
      i18nc("unit synonyms for matching user input",
            "pound-force per square inch;pound-force per square inch;psi"),
      ki18nc("amount in units (real)", "%1 pound-force per square inch"),
      ki18ncp("amount in units (integer)",
              "%1 pound-force per square inch", "%1 pound-force per square inch")
    );
    // http://en.wikipedia.org/wiki/InHg
    U(Pressure::InchesOfMercury, 3386.39,
      i18nc("pressure unit symbol", "inHg"),
      i18nc("unit description in lists", "inches of mercury"),
      i18nc("unit synonyms for matching user input",
            "inches of mercury;inches of mercury;inHg;in\""),
      ki18nc("amount in units (real)", "%1 inches of mercury"),
      ki18ncp("amount in units (integer)", "%1 inches of mercury", "%1 inches of mercury")
    );
}
