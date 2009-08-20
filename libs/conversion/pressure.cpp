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
#include "conversioni18ncatalog.h"
#include <KLocale>

Pressure::Pressure(QObject* parent)
: Conversion::UnitCategory(parent)
{
    Conversioni18nCatalog::loadCatalog();
    
    setObjectName("pressure");
    setName(i18n("Pressure"));

    addSIUnit("Pa", i18nc("pressure unit","pascal"), i18nc("pressure unit","pascals"));
    U(i18nc("pressure unit","bar"), i18nc("pressure unit","bars"), i18nc("pressure unit","bar"), 100000.0, );
    U(i18nc("pressure unit","millibar"), i18nc("pressure unit","millibars"),
        i18nc("pressure unit millibar","mbar"), 100.0, << "mb");
    U(i18nc("pressure unit","decibar"), i18nc("pressure unit","decibars"),
        i18nc("pressure unit decibar","dbar"), 10000.0, );
    U(i18nc("pressure unit","torr"), i18nc("pressure unit","torrs"),
        i18nc("pressure unit","torr"), 133.322, );
    U(i18nc("pressure unit","technical atmosphere"), i18nc("pressure unit","technical atmospheres"),
        i18nc("pressure unit technical atmosphere","at"), 98066.5, );
    U(i18nc("pressure unit","atmosphere"), i18nc("pressure unit","atmospheres"),
        i18nc("pressure unit atmosphere","atm"), 101325.0, );
    U(i18nc("pressure unit","pound-force per square inch"), i18nc("pressure unit","pound-force per square inch"),
        i18nc("pressure unit pound-force per square inch","psi"), 6894.76, );
    // http://en.wikipedia.org/wiki/InHg
    U(i18nc("pressure unit","inches of mercury"), i18nc("pressure unit","inches of mercury"),
        i18nc("pressure unit inches of mercury","inHg"), 3386.389, << "in\""); // at 0 °C
}
