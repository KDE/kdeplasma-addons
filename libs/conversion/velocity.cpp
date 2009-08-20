/*
 *   Copyright (C) 2008-2009 Petri Damstén <damu@iki.fi>
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

#include "velocity.h"
#include "conversioni18ncatalog.h"
#include <math.h>
#include <KLocale>

class Beaufort : public Conversion::Complex
{
    double toDefault(double value) const { return 0.836 * pow(value, 3.0 / 2.0); };
    double fromDefault(double value) const { return pow(value / 0.836, 2.0 / 3.0); };
};

Velocity::Velocity(QObject* parent)
: Conversion::UnitCategory(parent)
{
    Conversioni18nCatalog::loadCatalog();
    
    setObjectName("velocity");
    setName(i18n("Velocity"));

    setDefaultUnit(U(i18n("meter per second"), i18n("meters per second"), "m/s", 1.0, << "ms"));
    U(i18n("kilometer per hour"), i18n("kilometers per hour"), "km/h", 1.0 / 3.6, << "kmh");
    U(i18n("mile per hour"), i18n("miles per hour"), "mph", 0.44704, );
    U(i18n("foot per second"), i18n("feet per second"), i18n("ft/s"), 0.3048, << "ft/sec" << "fps");
    U(i18n("inch per second"), i18n("inches per second"), i18n("in/s"), 0.0254, << "in/sec" << "ips");
    U(i18n("knot"), i18n("knots"), "kt", 1.852/3.6, << i18n("nautical miles per hour"));
    // http://en.wikipedia.org/wiki/Speed_of_sound
    U(i18n("mach"), i18n("machs"), "Ma", 343.0, << i18n("speed of sound")); // In dry air at 20 °C
    U(i18n("speed of light"), i18n("speed of light"), "c", 299792458.0, );

    // http://en.wikipedia.org/wiki/Beaufort_scale
    U(i18n("beaufort"), i18n("beauforts"), "bft", new Beaufort(), );
}
