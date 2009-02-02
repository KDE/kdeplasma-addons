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

namespace Conversion
{

class Value::Private
{
public:
    Private(const QVariant& n = QVariant(), const QString& u = QString())
    : number(n)
    , unit(u)
    {
    };

    ~Private()
    {
    };

    QVariant number;
    QString unit;
    QString description;
};

Value::Value()
: d(new Value::Private())
{
}

Value::Value(const QVariant& n, const QString& u)
: d(new Value::Private(n, u))
{
}

Value::~Value()
{
   delete d;
}

bool Value::isValid() const
{
    return (d->number.isValid() && !d->unit.isEmpty());
}

QString Value::toString() const
{
    return d->number.toString() + ' ' + d->unit;
}

QVariant Value::number() const
{
    return d->number;
}

QString Value::unit() const
{
    return d->unit;
}

QString Value::description() const
{
    return d->description;
}

void Value::setDescription(const QString& description)
{
    d->description = description;
}

Value& Value::operator=(const Value& value)
{
    d->number = value.number();
    d->unit = value.unit();
    return *this;
}

}
