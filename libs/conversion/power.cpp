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

#include "power.h"
#include <KLocale>

Power::Power(QObject* parent)
: Conversion::UnitCategory(parent)
{
    setObjectName("power");
    setName(i18n("Power"));

    U(Power::Yottawatt, 1e+24,
      i18nc("power unit symbol", "YW"),
      i18nc("unit description in lists", "yottawatts"),
      i18nc("unit synonyms for matching user input", "yottawatt;yottawatts;YW"),
      ki18nc("amount in units (real)", "%1 yottawatts"),
      ki18ncp("amount in units (integer)", "%1 yottawatt", "%1 yottawatts")
    );
    U(Power::Zettawatt, 1e+21,
      i18nc("power unit symbol", "ZW"),
      i18nc("unit description in lists", "zettawatts"),
      i18nc("unit synonyms for matching user input", "zettawatt;zettawatts;ZW"),
      ki18nc("amount in units (real)", "%1 zettawatts"),
      ki18ncp("amount in units (integer)", "%1 zettawatt", "%1 zettawatts")
    );
    U(Power::Exawatt, 1e+18,
      i18nc("power unit symbol", "EW"),
      i18nc("unit description in lists", "exawatts"),
      i18nc("unit synonyms for matching user input", "exawatt;exawatts;EW"),
      ki18nc("amount in units (real)", "%1 exawatts"),
      ki18ncp("amount in units (integer)", "%1 exawatt", "%1 exawatts")
    );
    U(Power::Petawatt, 1e+15,
      i18nc("power unit symbol", "PW"),
      i18nc("unit description in lists", "petawatts"),
      i18nc("unit synonyms for matching user input", "petawatt;petawatts;PW"),
      ki18nc("amount in units (real)", "%1 petawatts"),
      ki18ncp("amount in units (integer)", "%1 petawatt", "%1 petawatts")
    );
    U(Power::Terawatt, 1e+12,
      i18nc("power unit symbol", "TW"),
      i18nc("unit description in lists", "terawatts"),
      i18nc("unit synonyms for matching user input", "terawatt;terawatts;TW"),
      ki18nc("amount in units (real)", "%1 terawatts"),
      ki18ncp("amount in units (integer)", "%1 terawatt", "%1 terawatts")
    );
    U(Power::Gigawatt, 1e+09,
      i18nc("power unit symbol", "GW"),
      i18nc("unit description in lists", "gigawatts"),
      i18nc("unit synonyms for matching user input", "gigawatt;gigawatts;GW"),
      ki18nc("amount in units (real)", "%1 gigawatts"),
      ki18ncp("amount in units (integer)", "%1 gigawatt", "%1 gigawatts")
    );
    U(Power::Megawatt, 1e+06,
      i18nc("power unit symbol", "MW"),
      i18nc("unit description in lists", "megawatts"),
      i18nc("unit synonyms for matching user input", "megawatt;megawatts;MW"),
      ki18nc("amount in units (real)", "%1 megawatts"),
      ki18ncp("amount in units (integer)", "%1 megawatt", "%1 megawatts")
    );
    U(Power::Kilowatt, 1000,
      i18nc("power unit symbol", "kW"),
      i18nc("unit description in lists", "kilowatts"),
      i18nc("unit synonyms for matching user input", "kilowatt;kilowatts;kW"),
      ki18nc("amount in units (real)", "%1 kilowatts"),
      ki18ncp("amount in units (integer)", "%1 kilowatt", "%1 kilowatts")
    );
    U(Power::Hectowatt, 100,
      i18nc("power unit symbol", "hW"),
      i18nc("unit description in lists", "hectowatts"),
      i18nc("unit synonyms for matching user input", "hectowatt;hectowatts;hW"),
      ki18nc("amount in units (real)", "%1 hectowatts"),
      ki18ncp("amount in units (integer)", "%1 hectowatt", "%1 hectowatts")
    );
    U(Power::Decawatt, 10,
      i18nc("power unit symbol", "daW"),
      i18nc("unit description in lists", "decawatts"),
      i18nc("unit synonyms for matching user input", "decawatt;decawatts;daW"),
      ki18nc("amount in units (real)", "%1 decawatts"),
      ki18ncp("amount in units (integer)", "%1 decawatt", "%1 decawatts")
    );
    setDefaultUnit(U(Power::Watt, 1,
      i18nc("power unit symbol", "W"),
      i18nc("unit description in lists", "watts"),
      i18nc("unit synonyms for matching user input", "watt;watts;W"),
      ki18nc("amount in units (real)", "%1 watts"),
      ki18ncp("amount in units (integer)", "%1 watt", "%1 watts")
    ));
    U(Power::Deciwatt, 0.1,
      i18nc("power unit symbol", "dW"),
      i18nc("unit description in lists", "deciwatts"),
      i18nc("unit synonyms for matching user input", "deciwatt;deciwatts;dW"),
      ki18nc("amount in units (real)", "%1 deciwatts"),
      ki18ncp("amount in units (integer)", "%1 deciwatt", "%1 deciwatts")
    );
    U(Power::Centiwatt, 0.01,
      i18nc("power unit symbol", "cW"),
      i18nc("unit description in lists", "centiwatts"),
      i18nc("unit synonyms for matching user input", "centiwatt;centiwatts;cW"),
      ki18nc("amount in units (real)", "%1 centiwatts"),
      ki18ncp("amount in units (integer)", "%1 centiwatt", "%1 centiwatts")
    );
    U(Power::Milliwatt, 0.001,
      i18nc("power unit symbol", "mW"),
      i18nc("unit description in lists", "milliwatts"),
      i18nc("unit synonyms for matching user input", "milliwatt;milliwatts;mW"),
      ki18nc("amount in units (real)", "%1 milliwatts"),
      ki18ncp("amount in units (integer)", "%1 milliwatt", "%1 milliwatts")
    );
    U(Power::Microwatt, 1e-06,
      i18nc("power unit symbol", "µW"),
      i18nc("unit description in lists", "microwatts"),
      i18nc("unit synonyms for matching user input", "microwatt;microwatts;µW;uW"),
      ki18nc("amount in units (real)", "%1 microwatts"),
      ki18ncp("amount in units (integer)", "%1 microwatt", "%1 microwatts")
    );
    U(Power::Nanowatt, 1e-09,
      i18nc("power unit symbol", "nW"),
      i18nc("unit description in lists", "nanowatts"),
      i18nc("unit synonyms for matching user input", "nanowatt;nanowatts;nW"),
      ki18nc("amount in units (real)", "%1 nanowatts"),
      ki18ncp("amount in units (integer)", "%1 nanowatt", "%1 nanowatts")
    );
    U(Power::Picowatt, 1e-12,
      i18nc("power unit symbol", "pW"),
      i18nc("unit description in lists", "picowatts"),
      i18nc("unit synonyms for matching user input", "picowatt;picowatts;pW"),
      ki18nc("amount in units (real)", "%1 picowatts"),
      ki18ncp("amount in units (integer)", "%1 picowatt", "%1 picowatts")
    );
    U(Power::Femtowatt, 1e-15,
      i18nc("power unit symbol", "fW"),
      i18nc("unit description in lists", "femtowatts"),
      i18nc("unit synonyms for matching user input", "femtowatt;femtowatts;fW"),
      ki18nc("amount in units (real)", "%1 femtowatts"),
      ki18ncp("amount in units (integer)", "%1 femtowatt", "%1 femtowatts")
    );
    U(Power::Attowatt, 1e-18,
      i18nc("power unit symbol", "aW"),
      i18nc("unit description in lists", "attowatts"),
      i18nc("unit synonyms for matching user input", "attowatt;attowatts;aW"),
      ki18nc("amount in units (real)", "%1 attowatts"),
      ki18ncp("amount in units (integer)", "%1 attowatt", "%1 attowatts")
    );
    U(Power::Zeptowatt, 1e-21,
      i18nc("power unit symbol", "zW"),
      i18nc("unit description in lists", "zeptowatts"),
      i18nc("unit synonyms for matching user input", "zeptowatt;zeptowatts;zW"),
      ki18nc("amount in units (real)", "%1 zeptowatts"),
      ki18ncp("amount in units (integer)", "%1 zeptowatt", "%1 zeptowatts")
    );
    U(Power::Yoctowatt, 1e-24,
      i18nc("power unit symbol", "yW"),
      i18nc("unit description in lists", "yoctowatts"),
      i18nc("unit synonyms for matching user input", "yoctowatt;yoctowatts;yW"),
      ki18nc("amount in units (real)", "%1 yoctowatts"),
      ki18ncp("amount in units (integer)", "%1 yoctowatt", "%1 yoctowatts")
    );
    U(Power::Horsepower, 735.499,
      i18nc("power unit symbol", "hp"),
      i18nc("unit description in lists", "horsepowers"),
      i18nc("unit synonyms for matching user input", "horsepower;horsepowers;hp"),
      ki18nc("amount in units (real)", "%1 horsepowers"),
      ki18ncp("amount in units (integer)", "%1 horsepower", "%1 horsepowers")
    );
}
