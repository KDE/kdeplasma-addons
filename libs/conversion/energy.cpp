/*
 * Copyright (C) 2009 Petri Damstén <damu@iki.fi>
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

#include "energy.h"
#include <KLocale>

Energy::Energy(QObject* parent)
: Conversion::UnitCategory(parent)
{
    setObjectName("energy");
    setName(i18n("Energy"));
    setDefaultUnit("J");
    addSIUnit(defaultUnit(), i18n("joule"), i18n("joules"));

    U(i18n("electronvolt"), i18n("electronvolts"), "eV", 1.60217653E-19, );
    U(i18n("rydberg"), i18n("rydbergs"), "Ry", 2.179872E-18, );
    U(i18n("kilocalorie"), i18n("kilocalories"), "kcal", 4186.8, );
}
