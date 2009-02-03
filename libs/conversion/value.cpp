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

#include "value.h"
#include "converter.h"
#include <KDebug>

namespace Conversion
{

class Value::Private
{
public:
    Private(double n = 0.0, const Unit* u = 0)
    : number(n)
    , unit(u)
    {
    };

    ~Private()
    {
    };

    double number;
    const Unit* unit;
};

Value::Value()
: d(new Value::Private())
{
}

Value::Value(double n, const Unit* u)
: d(new Value::Private(n, u))
{
}

Value::Value(double n, const QString& u)
: d(new Value::Private(n, Conversion::Converter::self()->unit(u)))
{
}

Value::Value(const QVariant& n, const QString& u)
: d(new Value::Private(n.toDouble(), Conversion::Converter::self()->unit(u)))
{
}

Value::~Value()
{
    delete d;
}

bool Value::isValid() const
{
    return (d->unit != 0 && d->unit->isValid());
}

QString Value::toString() const
{
    if (isValid()) {
        return QString("%1 %2").arg(d->number).arg(d->unit->toString(d->number));
    }
    return QString();
}

double Value::number() const
{
    return d->number;
}

const Unit* Value::unit() const
{
    if (!d->unit) {
        d->unit = new Unit(Conversion::Converter::self());
    }
    return d->unit;
}

Value& Value::operator=(const Value& value)
{
    d->number = value.number();
    d->unit = value.unit();
    return *this;
}

}
