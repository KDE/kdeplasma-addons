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

#include "converter.h"
#include "unit.h"

#include "length.h"
#include "area.h"
#include "volume.h"
#include "temperature.h"
#include "velocity.h"
#include "mass.h"
#include "pressure.h"
#include "energy.h"
#include "currency.h"
#include "power.h"
#include "timeunit.h"
#include "fuel_efficiency.h"
#include <KGlobal>
#include <KDebug>

namespace Conversion
{

class Converter::Private
{
public:
    Private()
    {
    };

    ~Private()
    {
    };
};

class ConverterSingleton
{
    public:
        Converter self;
};

K_GLOBAL_STATIC(ConverterSingleton, s_instance)

Converter::Converter(QObject* parent)
: QObject(parent)
, d(/*new Converter::Private*/0)
{
    new Length(this);
    new Area(this);
    new Volume(this);
    new Temperature(this);
    new Velocity(this);
    new Mass(this);
    new Pressure(this);
    new Energy(this);
    new Currency(this);
    new Power(this);
    new Time(this);
    new FuelEfficiency(this);
}

Converter::~Converter()
{
    //delete d;
}

Converter* Converter::self()
{
    return &s_instance->self;
}

Value Converter::convert(const Value& value, const QString& toUnit) const
{
    UnitCategory* category = value.unit()->category();
    if (!category) {
        return Value();
    }
    return category->convert(value, toUnit);
}

UnitCategory* Converter::categoryForUnit(const QString& unit) const
{
    foreach (UnitCategory* u, categories()) {
        if (u->hasUnit(unit)) {
            return u;
        }
    }
    return 0;
}

Unit* Converter::unit(const QString& unit) const
{
    foreach (UnitCategory* u, categories()) {
        Unit* unitClass = u->unit(unit);
        if (unitClass) {
            return unitClass;
        }
    }
    return 0;
}

UnitCategory* Converter::category(const QString& category) const
{
    QList<UnitCategory*> units = findChildren<UnitCategory*>(category);
    if (!units.isEmpty()) {
        return units[0];
    }
    return 0;
}

QList<UnitCategory*> Converter::categories() const
{
    return findChildren<UnitCategory*>();
}

}

#include "converter.moc"
