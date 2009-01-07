/*
 * Copyright (C) 2008 Petri Damstén <damu@iki.fi>
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

#include "converter.h"
#include "unit.h"

#include "length.h"
#include "area.h"
#include "volume.h"
#include "temperature.h"
#include "speed.h"
#include "mass.h"

#include <KGlobal>

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
    new Speed(this);
    new Mass(this);
}

Converter::~Converter()
{
    //delete d;
}

Converter* Converter::self()
{
    return &s_instance->self;
}

Value Converter::convert(const Value& value, const QString& toUnit)
{
    UnitCategory* unit = categoryForUnit(value.unit());
    if (!unit) {
        return Value();
    }
    if (!toUnit.isEmpty() && !unit->hasUnit(toUnit)) {
        return Value();
    }
    return unit->convert(value, toUnit);
}

UnitCategory* Converter::categoryForUnit(const QString& unit)
{
    foreach (UnitCategory* u, categories()) {
        if (u->hasUnit(unit)) {
            return u;
        }
    }
    return 0;
}

UnitCategory* Converter::category(const QString& category)
{
    QList<UnitCategory*> units = findChildren<UnitCategory*>(category);
    if (!units.isEmpty()) {
        return units[0];
    }
    return 0;
}

QList<UnitCategory*> Converter::categories()
{
    return findChildren<UnitCategory*>();
}

}

#include "converter.moc"
