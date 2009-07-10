/*
 *   Copyright (C) 2008-2009 Petri Damstén <damu@iki.fi>
 *   Copyright (C) 2008 Kristof Bal <kristof.bal@gmail.com>
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

#include "mass.h"
#include <KLocale>

Mass::Mass(QObject* parent)
: Conversion::UnitCategory(parent)
{
    setObjectName("mass");
    setName(i18n("Mass"));

    U(Mass::Yottagram, 1e+24,
      i18nc("mass unit symbol", "Yg"),
      i18nc("unit description in lists", "yottagrams"),
      i18nc("unit synonyms for matching user input", "yottagram;yottagrams;Yg"),
      ki18nc("amount in units (real)", "%1 yottagrams"),
      ki18ncp("amount in units (integer)", "%1 yottagram", "%1 yottagrams")
    );
    U(Mass::Zettagram, 1e+21,
      i18nc("mass unit symbol", "Zg"),
      i18nc("unit description in lists", "zettagrams"),
      i18nc("unit synonyms for matching user input", "zettagram;zettagrams;Zg"),
      ki18nc("amount in units (real)", "%1 zettagrams"),
      ki18ncp("amount in units (integer)", "%1 zettagram", "%1 zettagrams")
    );
    U(Mass::Exagram, 1e+18,
      i18nc("mass unit symbol", "Eg"),
      i18nc("unit description in lists", "exagrams"),
      i18nc("unit synonyms for matching user input", "exagram;exagrams;Eg"),
      ki18nc("amount in units (real)", "%1 exagrams"),
      ki18ncp("amount in units (integer)", "%1 exagram", "%1 exagrams")
    );
    U(Mass::Petagram, 1e+15,
      i18nc("mass unit symbol", "Pg"),
      i18nc("unit description in lists", "petagrams"),
      i18nc("unit synonyms for matching user input", "petagram;petagrams;Pg"),
      ki18nc("amount in units (real)", "%1 petagrams"),
      ki18ncp("amount in units (integer)", "%1 petagram", "%1 petagrams")
    );
    U(Mass::Teragram, 1e+12,
      i18nc("mass unit symbol", "Tg"),
      i18nc("unit description in lists", "teragrams"),
      i18nc("unit synonyms for matching user input", "teragram;teragrams;Tg"),
      ki18nc("amount in units (real)", "%1 teragrams"),
      ki18ncp("amount in units (integer)", "%1 teragram", "%1 teragrams")
    );
    U(Mass::Gigagram, 1e+09,
      i18nc("mass unit symbol", "Gg"),
      i18nc("unit description in lists", "gigagrams"),
      i18nc("unit synonyms for matching user input", "gigagram;gigagrams;Gg"),
      ki18nc("amount in units (real)", "%1 gigagrams"),
      ki18ncp("amount in units (integer)", "%1 gigagram", "%1 gigagrams")
    );
    U(Mass::Megagram, 1e+06,
      i18nc("mass unit symbol", "Mg"),
      i18nc("unit description in lists", "megagrams"),
      i18nc("unit synonyms for matching user input", "megagram;megagrams;Mg"),
      ki18nc("amount in units (real)", "%1 megagrams"),
      ki18ncp("amount in units (integer)", "%1 megagram", "%1 megagrams")
    );
    setDefaultUnit(U(Mass::Kilogram, 1000,
      i18nc("mass unit symbol", "kg"),
      i18nc("unit description in lists", "kilograms"),
      i18nc("unit synonyms for matching user input", "kilogram;kilograms;kg"),
      ki18nc("amount in units (real)", "%1 kilograms"),
      ki18ncp("amount in units (integer)", "%1 kilogram", "%1 kilograms")
    ));
    U(Mass::Hectogram, 100,
      i18nc("mass unit symbol", "hg"),
      i18nc("unit description in lists", "hectograms"),
      i18nc("unit synonyms for matching user input", "hectogram;hectograms;hg"),
      ki18nc("amount in units (real)", "%1 hectograms"),
      ki18ncp("amount in units (integer)", "%1 hectogram", "%1 hectograms")
    );
    U(Mass::Decagram, 10,
      i18nc("mass unit symbol", "dag"),
      i18nc("unit description in lists", "decagrams"),
      i18nc("unit synonyms for matching user input", "decagram;decagrams;dag"),
      ki18nc("amount in units (real)", "%1 decagrams"),
      ki18ncp("amount in units (integer)", "%1 decagram", "%1 decagrams")
    );
    U(Mass::Gram, 1,
      i18nc("mass unit symbol", "g"),
      i18nc("unit description in lists", "grams"),
      i18nc("unit synonyms for matching user input", "gram;grams;g"),
      ki18nc("amount in units (real)", "%1 grams"),
      ki18ncp("amount in units (integer)", "%1 gram", "%1 grams")
    );
    U(Mass::Decigram, 0.1,
      i18nc("mass unit symbol", "dg"),
      i18nc("unit description in lists", "decigrams"),
      i18nc("unit synonyms for matching user input", "decigram;decigrams;dg"),
      ki18nc("amount in units (real)", "%1 decigrams"),
      ki18ncp("amount in units (integer)", "%1 decigram", "%1 decigrams")
    );
    U(Mass::Centigram, 0.01,
      i18nc("mass unit symbol", "cg"),
      i18nc("unit description in lists", "centigrams"),
      i18nc("unit synonyms for matching user input", "centigram;centigrams;cg"),
      ki18nc("amount in units (real)", "%1 centigrams"),
      ki18ncp("amount in units (integer)", "%1 centigram", "%1 centigrams")
    );
    U(Mass::Milligram, 0.001,
      i18nc("mass unit symbol", "mg"),
      i18nc("unit description in lists", "milligrams"),
      i18nc("unit synonyms for matching user input", "milligram;milligrams;mg"),
      ki18nc("amount in units (real)", "%1 milligrams"),
      ki18ncp("amount in units (integer)", "%1 milligram", "%1 milligrams")
    );
    U(Mass::Microgram, 1e-06,
      i18nc("mass unit symbol", "µg"),
      i18nc("unit description in lists", "micrograms"),
      i18nc("unit synonyms for matching user input", "microgram;micrograms;µg;ug"),
      ki18nc("amount in units (real)", "%1 micrograms"),
      ki18ncp("amount in units (integer)", "%1 microgram", "%1 micrograms")
    );
    U(Mass::Nanogram, 1e-09,
      i18nc("mass unit symbol", "ng"),
      i18nc("unit description in lists", "nanograms"),
      i18nc("unit synonyms for matching user input", "nanogram;nanograms;ng"),
      ki18nc("amount in units (real)", "%1 nanograms"),
      ki18ncp("amount in units (integer)", "%1 nanogram", "%1 nanograms")
    );
    U(Mass::Picogram, 1e-12,
      i18nc("mass unit symbol", "pg"),
      i18nc("unit description in lists", "picograms"),
      i18nc("unit synonyms for matching user input", "picogram;picograms;pg"),
      ki18nc("amount in units (real)", "%1 picograms"),
      ki18ncp("amount in units (integer)", "%1 picogram", "%1 picograms")
    );
    U(Mass::Femtogram, 1e-15,
      i18nc("mass unit symbol", "fg"),
      i18nc("unit description in lists", "femtograms"),
      i18nc("unit synonyms for matching user input", "femtogram;femtograms;fg"),
      ki18nc("amount in units (real)", "%1 femtograms"),
      ki18ncp("amount in units (integer)", "%1 femtogram", "%1 femtograms")
    );
    U(Mass::Attogram, 1e-18,
      i18nc("mass unit symbol", "ag"),
      i18nc("unit description in lists", "attograms"),
      i18nc("unit synonyms for matching user input", "attogram;attograms;ag"),
      ki18nc("amount in units (real)", "%1 attograms"),
      ki18ncp("amount in units (integer)", "%1 attogram", "%1 attograms")
    );
    U(Mass::Zeptogram, 1e-21,
      i18nc("mass unit symbol", "zg"),
      i18nc("unit description in lists", "zeptograms"),
      i18nc("unit synonyms for matching user input", "zeptogram;zeptograms;zg"),
      ki18nc("amount in units (real)", "%1 zeptograms"),
      ki18ncp("amount in units (integer)", "%1 zeptogram", "%1 zeptograms")
    );
    U(Mass::Yoctogram, 1e-24,
      i18nc("mass unit symbol", "yg"),
      i18nc("unit description in lists", "yoctograms"),
      i18nc("unit synonyms for matching user input", "yoctogram;yoctograms;yg"),
      ki18nc("amount in units (real)", "%1 yoctograms"),
      ki18ncp("amount in units (integer)", "%1 yoctogram", "%1 yoctograms")
    );
    U(Mass::Ton, 1e+06,
      i18nc("mass unit symbol", "t"),
      i18nc("unit description in lists", "tons"),
      i18nc("unit synonyms for matching user input", "ton;tons;t;tonne"),
      ki18nc("amount in units (real)", "%1 tons"),
      ki18ncp("amount in units (integer)", "%1 ton", "%1 tons")
    );
    //I guess it's useful...
    U(Mass::Carat, 0.2,
      i18nc("mass unit symbol", "CD"),
      i18nc("unit description in lists", "carats"),
      i18nc("unit synonyms for matching user input", "carat;carats;CD"),
      ki18nc("amount in units (real)", "%1 carats"),
      ki18ncp("amount in units (integer)", "%1 carat", "%1 carats")
    );
    //http://en.wikipedia.org/wiki/Pound_(mass)#International_pound
    U(Mass::Pound, 453.592,
      i18nc("mass unit symbol", "lb"),
      i18nc("unit description in lists", "pounds"),
      i18nc("unit synonyms for matching user input", "pound;pounds;lb"),
      ki18nc("amount in units (real)", "%1 pounds"),
      ki18ncp("amount in units (integer)", "%1 pound", "%1 pounds")
    );
    //International avoirdupois ounce
    U(Mass::Ounce, 28.3495,
      i18nc("mass unit symbol", "oz"),
      i18nc("unit description in lists", "ounces"),
      i18nc("unit synonyms for matching user input", "ounce;ounces;oz"),
      ki18nc("amount in units (real)", "%1 ounces"),
      ki18ncp("amount in units (integer)", "%1 ounce", "%1 ounces")
    );
    U(Mass::TroyOunce, 31.1035,
      i18nc("mass unit symbol", "t oz"),
      i18nc("unit description in lists", "troy ounces"),
      i18nc("unit synonyms for matching user input", "troy ounce;troy ounces;t oz"),
      ki18nc("amount in units (real)", "%1 troy ounces"),
      ki18ncp("amount in units (integer)", "%1 troy ounce", "%1 troy ounces")
    );
    U(Mass::Newton, 101.937,
      i18nc("mass unit symbol", "N"),
      i18nc("unit description in lists", "newtons"),
      i18nc("unit synonyms for matching user input", "newton;newtons;N"),
      ki18nc("amount in units (real)", "%1 newtons"),
      ki18ncp("amount in units (integer)", "%1 newton", "%1 newtons")
    );
    //used a lot in industry (aircraft engines for example)
    U(Mass::Kilonewton, 101937,
      i18nc("mass unit symbol", "kN"),
      i18nc("unit description in lists", "kilonewton"),
      i18nc("unit synonyms for matching user input", "kilonewton;kilonewton;kN"),
      ki18nc("amount in units (real)", "%1 kilonewton"),
      ki18ncp("amount in units (integer)", "%1 kilonewton", "%1 kilonewton")
    );
}
