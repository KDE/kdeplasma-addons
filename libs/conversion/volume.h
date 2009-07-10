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

#ifndef VOLUME_H
#define VOLUME_H

#include "unitcategory.h"

class Volume : public Conversion::UnitCategory
{
public:
    enum Id {
        CubicYottameter = 3000, CubicZettameter, CubicExameter, CubicPetameter, CubicTerameter,
        CubicGigameter, CubicMegameter, CubicKilometer, CubicHectometer, CubicDecameter, CubicMeter,
        CubicDecimeter, CubicCentimeter, CubicMillimeter, CubicMicrometer, CubicNanometer,
        CubicPicometer, CubicFemtometer, CubicAttometer, CubicZeptometer, CubicYoctometer,
        Yottaliter, Zettaliter, Exaliter, Petaliter, Teraliter, Gigaliter, Megaliter, Kiloliter,
        Hectoliter, Decaliter, Liter, Deciliter, Centiliter, Milliliter, Microliter, Nanoliter,
        Picoliter, Femtoliter, Attoliter, Zeptoliter, Yoctoliter, CubicFoot, CubicInch, CubicMile,
        FluidOunce, Cup, GallonUS, PintImperial
    };

    Volume(QObject* parent = 0);
};

#endif
