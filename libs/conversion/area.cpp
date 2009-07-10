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
#include <KLocale>

Area::Area(QObject* parent)
: Conversion::UnitCategory(parent)
{
    setObjectName("area");
    setName(i18n("Area"));

    U(Area::SquareYottameter, 1e+48,
      i18nc("area unit symbol", "Ym²"),
      i18nc("unit description in lists", "square yottameters"),
      i18nc("unit synonyms for matching user input",
            "square yottameter;square yottameters;Ym²;Ym/-2;Ym^2;Ym2"),
      ki18nc("amount in units (real)", "%1 square yottameters"),
      ki18ncp("amount in units (integer)", "%1 square yottameter", "%1 square yottameters")
    );
    U(Area::SquareZettameter, 1e+42,
      i18nc("area unit symbol", "Zm²"),
      i18nc("unit description in lists", "square zettameters"),
      i18nc("unit synonyms for matching user input",
            "square zettameter;square zettameters;Zm²;Zm/-2;Zm^2;Zm2"),
      ki18nc("amount in units (real)", "%1 square zettameters"),
      ki18ncp("amount in units (integer)", "%1 square zettameter", "%1 square zettameters")
    );
    U(Area::SquareExameter, 1e+36,
      i18nc("area unit symbol", "Em²"),
      i18nc("unit description in lists", "square exameters"),
      i18nc("unit synonyms for matching user input",
            "square exameter;square exameters;Em²;Em/-2;Em^2;Em2"),
      ki18nc("amount in units (real)", "%1 square exameters"),
      ki18ncp("amount in units (integer)", "%1 square exameter", "%1 square exameters")
    );
    U(Area::SquarePetameter, 1e+30,
      i18nc("area unit symbol", "Pm²"),
      i18nc("unit description in lists", "square petameters"),
      i18nc("unit synonyms for matching user input",
            "square petameter;square petameters;Pm²;Pm/-2;Pm^2;Pm2"),
      ki18nc("amount in units (real)", "%1 square petameters"),
      ki18ncp("amount in units (integer)", "%1 square petameter", "%1 square petameters")
    );
    U(Area::SquareTerameter, 1e+24,
      i18nc("area unit symbol", "Tm²"),
      i18nc("unit description in lists", "square terameters"),
      i18nc("unit synonyms for matching user input",
            "square terameter;square terameters;Tm²;Tm/-2;Tm^2;Tm2"),
      ki18nc("amount in units (real)", "%1 square terameters"),
      ki18ncp("amount in units (integer)", "%1 square terameter", "%1 square terameters")
    );
    U(Area::SquareGigameter, 1e+18,
      i18nc("area unit symbol", "Gm²"),
      i18nc("unit description in lists", "square gigameters"),
      i18nc("unit synonyms for matching user input",
            "square gigameter;square gigameters;Gm²;Gm/-2;Gm^2;Gm2"),
      ki18nc("amount in units (real)", "%1 square gigameters"),
      ki18ncp("amount in units (integer)", "%1 square gigameter", "%1 square gigameters")
    );
    U(Area::SquareMegameter, 1e+12,
      i18nc("area unit symbol", "Mm²"),
      i18nc("unit description in lists", "square megameters"),
      i18nc("unit synonyms for matching user input",
            "square megameter;square megameters;Mm²;Mm/-2;Mm^2;Mm2"),
      ki18nc("amount in units (real)", "%1 square megameters"),
      ki18ncp("amount in units (integer)", "%1 square megameter", "%1 square megameters")
    );
    U(Area::SquareKilometer, 1e+06,
      i18nc("area unit symbol", "km²"),
      i18nc("unit description in lists", "square kilometers"),
      i18nc("unit synonyms for matching user input",
            "square kilometer;square kilometers;km²;km/-2;km^2;km2"),
      ki18nc("amount in units (real)", "%1 square kilometers"),
      ki18ncp("amount in units (integer)", "%1 square kilometer", "%1 square kilometers")
    );
    U(Area::SquareHectometer, 10000,
      i18nc("area unit symbol", "hm²"),
      i18nc("unit description in lists", "square hectometers"),
      i18nc("unit synonyms for matching user input",
            "square hectometer;square hectometers;hm²;hm/-2;hm^2;hm2;hectare;hectares"),
      ki18nc("amount in units (real)", "%1 square hectometers"),
      ki18ncp("amount in units (integer)", "%1 square hectometer", "%1 square hectometers")
    );
    U(Area::SquareDecameter, 100,
      i18nc("area unit symbol", "dam²"),
      i18nc("unit description in lists", "square decameters"),
      i18nc("unit synonyms for matching user input",
            "square decameter;square decameters;dam²;dam/-2;dam^2;dam2"),
      ki18nc("amount in units (real)", "%1 square decameters"),
      ki18ncp("amount in units (integer)", "%1 square decameter", "%1 square decameters")
    );
    setDefaultUnit(U(Area::SquareMeter, 1,
      i18nc("area unit symbol", "m²"),
      i18nc("unit description in lists", "square meters"),
      i18nc("unit synonyms for matching user input", "square meter;square meters;m²;m/-2;m^2;m2"),
      ki18nc("amount in units (real)", "%1 square meters"),
      ki18ncp("amount in units (integer)", "%1 square meter", "%1 square meters")
    ));
    U(Area::SquareDecimeter, 0.01,
      i18nc("area unit symbol", "dm²"),
      i18nc("unit description in lists", "square decimeters"),
      i18nc("unit synonyms for matching user input",
            "square decimeter;square decimeters;dm²;dm/-2;dm^2;dm2"),
      ki18nc("amount in units (real)", "%1 square decimeters"),
      ki18ncp("amount in units (integer)", "%1 square decimeter", "%1 square decimeters")
    );
    U(Area::SquareCentimeter, 0.0001,
      i18nc("area unit symbol", "cm²"),
      i18nc("unit description in lists", "square centimeters"),
      i18nc("unit synonyms for matching user input",
            "square centimeter;square centimeters;cm²;cm/-2;cm^2;cm2"),
      ki18nc("amount in units (real)", "%1 square centimeters"),
      ki18ncp("amount in units (integer)", "%1 square centimeter", "%1 square centimeters")
    );
    U(Area::SquareMillimeter, 1e-06,
      i18nc("area unit symbol", "mm²"),
      i18nc("unit description in lists", "square millimeters"),
      i18nc("unit synonyms for matching user input",
            "square millimeter;square millimeters;mm²;mm/-2;mm^2;mm2"),
      ki18nc("amount in units (real)", "%1 square millimeters"),
      ki18ncp("amount in units (integer)", "%1 square millimeter", "%1 square millimeters")
    );
    U(Area::SquareMicrometer, 1e-12,
      i18nc("area unit symbol", "µm²"),
      i18nc("unit description in lists", "square micrometers"),
      i18nc("unit synonyms for matching user input",
            "square micrometer;square micrometers;µm²;um²;µm/-2;µm^2;µm2"),
      ki18nc("amount in units (real)", "%1 square micrometers"),
      ki18ncp("amount in units (integer)", "%1 square micrometer", "%1 square micrometers")
    );
    U(Area::SquareNanometer, 1e-18,
      i18nc("area unit symbol", "nm²"),
      i18nc("unit description in lists", "square nanometers"),
      i18nc("unit synonyms for matching user input",
            "square nanometer;square nanometers;nm²;nm/-2;nm^2;nm2"),
      ki18nc("amount in units (real)", "%1 square nanometers"),
      ki18ncp("amount in units (integer)", "%1 square nanometer", "%1 square nanometers")
    );
    U(Area::SquarePicometer, 1e-24,
      i18nc("area unit symbol", "pm²"),
      i18nc("unit description in lists", "square picometers"),
      i18nc("unit synonyms for matching user input",
            "square picometer;square picometers;pm²;pm/-2;pm^2;pm2"),
      ki18nc("amount in units (real)", "%1 square picometers"),
      ki18ncp("amount in units (integer)", "%1 square picometer", "%1 square picometers")
    );
    U(Area::SquareFemtometer, 1e-30,
      i18nc("area unit symbol", "fm²"),
      i18nc("unit description in lists", "square femtometers"),
      i18nc("unit synonyms for matching user input",
            "square femtometer;square femtometers;fm²;fm/-2;fm^2;fm2"),
      ki18nc("amount in units (real)", "%1 square femtometers"),
      ki18ncp("amount in units (integer)", "%1 square femtometer", "%1 square femtometers")
    );
    U(Area::SquareAttometer, 1e-36,
      i18nc("area unit symbol", "am²"),
      i18nc("unit description in lists", "square attometers"),
      i18nc("unit synonyms for matching user input",
            "square attometer;square attometers;am²;am/-2;am^2;am2"),
      ki18nc("amount in units (real)", "%1 square attometers"),
      ki18ncp("amount in units (integer)", "%1 square attometer", "%1 square attometers")
    );
    U(Area::SquareZeptometer, 1e-42,
      i18nc("area unit symbol", "zm²"),
      i18nc("unit description in lists", "square zeptometers"),
      i18nc("unit synonyms for matching user input",
            "square zeptometer;square zeptometers;zm²;zm/-2;zm^2;zm2"),
      ki18nc("amount in units (real)", "%1 square zeptometers"),
      ki18ncp("amount in units (integer)", "%1 square zeptometer", "%1 square zeptometers")
    );
    U(Area::SquareYoctometer, 1e-48,
      i18nc("area unit symbol", "ym²"),
      i18nc("unit description in lists", "square yoctometers"),
      i18nc("unit synonyms for matching user input",
            "square yoctometer;square yoctometers;ym²;ym/-2;ym^2;ym2"),
      ki18nc("amount in units (real)", "%1 square yoctometers"),
      ki18ncp("amount in units (integer)", "%1 square yoctometer", "%1 square yoctometers")
    );
    U(Area::Acre, 4046.86,
      i18nc("area unit symbol", "acre"),
      i18nc("unit description in lists", "acres"),
      i18nc("unit synonyms for matching user input", "acre;acres;acre"),
      ki18nc("amount in units (real)", "%1 acres"),
      ki18ncp("amount in units (integer)", "%1 acre", "%1 acres")
    );
    U(Area::SquareFoot, 0.092903,
      i18nc("area unit symbol", "ft²"),
      i18nc("unit description in lists", "square feet"),
      i18nc("unit synonyms for matching user input",
            "square foot;square feet;ft²;square ft;sq foot;sq ft;sq feet;feet²"),
      ki18nc("amount in units (real)", "%1 square feet"),
      ki18ncp("amount in units (integer)", "%1 square foot", "%1 square feet")
    );
    U(Area::SquareInch, 0.00064516,
      i18nc("area unit symbol", "in²"),
      i18nc("unit description in lists", "square inches"),
      i18nc("unit synonyms for matching user input",
            "square inch;square inches;in²;square inch;square in;sq inches;sq inch;sq in;inch²"),
      ki18nc("amount in units (real)", "%1 square inches"),
      ki18ncp("amount in units (integer)", "%1 square inch", "%1 square inches")
    );
    U(Area::SquareMile, 2.58999e+06,
      i18nc("area unit symbol", "mi²"),
      i18nc("unit description in lists", "square miles"),
      i18nc("unit synonyms for matching user input",
            "square mile;square miles;mi²;square mi;sq miles;sq mile;sq mi;mile²"),
      ki18nc("amount in units (real)", "%1 square miles"),
      ki18ncp("amount in units (integer)", "%1 square mile", "%1 square miles")
    );
}
