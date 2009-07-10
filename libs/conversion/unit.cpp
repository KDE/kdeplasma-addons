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

#include "unit.h"
#include "unitcategory.h"
#include <QTimer>
#include <KLocale>
#include <KDebug>

namespace Conversion
{

class Unit::Private
{
public:
    Private()
    : multiplier(1.0)
    , complex(0)
    {
    };

    ~Private()
    {
        delete complex;
    };

    QString symbol;
    QString description;
    double multiplier;
    KLocalizedString real;
    KLocalizedString integer;
    const Complex* complex;
};

Unit::Unit(QObject* parent)
: QObject(parent)
, d(new Unit::Private)
{
}

Unit::Unit(QObject* parent, int id, double multiplier, const QString& symbol,
           const QString& description, const QString& match,
           const KLocalizedString& real, const KLocalizedString& integer)
: QObject(parent)
, d(new Unit::Private)
{
    UnitCategory* uc = category();
    if (uc) {
        uc->addUnitMapValues(this, match);
        uc->addIdMapValue(this, id);
    }
    d->multiplier = multiplier;
    d->real = real;
    d->integer = integer;
    d->symbol = symbol;
    d->description = description;
}

Unit::Unit(QObject* parent, int id, const Complex* complex, const QString& symbol,
           const QString& description, const QString& match,
           const KLocalizedString& real, const KLocalizedString& integer)
: QObject(parent)
, d(new Unit::Private)
{
    UnitCategory* uc = category();
    if (uc) {
        uc->addUnitMapValues(this, match);
        uc->addIdMapValue(this, id);
    }
    d->complex = complex;
    d->real = real;
    d->integer = integer;
    d->symbol = symbol;
    d->description = description;
}

Unit::~Unit()
{
    delete d;
}

UnitCategory* Unit::category() const
{
    return dynamic_cast<UnitCategory*>(parent());
}

QString Unit::description() const
{
    return d->description;
}

QString Unit::symbol() const
{
    return d->symbol;
}

double Unit::multiplier() const
{
    return d->multiplier;
}

void Unit::setMultiplier(double multiplier)
{
    d->multiplier = multiplier;
}

double Unit::toDefault(double value) const
{
    if (d->complex) {
        return d->complex->toDefault(value);
    } else {
        return value * d->multiplier;
    }
}

double Unit::fromDefault(double value) const
{
    if (d->complex) {
        return d->complex->fromDefault(value);
    } else {
        return value / d->multiplier;
    }
}

QString Unit::toString(double value) const
{
    if ((int)value == value) {
        return d->integer.subs((int)value).toString();
    }
    return d->real.subs(value).toString();
}

bool Unit::isValid() const
{
    return !d->symbol.isEmpty();
}

}

#include "unit.moc"
