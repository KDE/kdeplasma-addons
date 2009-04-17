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

    QString singular;
    QString plural;
    QString symbol;
    double multiplier;
    const Complex* complex;
};

Unit::Unit(QObject* parent)
: QObject(parent)
, d(new Unit::Private)
{
}

Unit::Unit(QObject* parent, const QString& singular, const QString& plural, const QString& symbol,
           double multiplier, const QStringList& synonyms)
: QObject(parent)
, d(new Unit::Private)
{
    UnitCategory* uc = category();
    if (uc) {
        uc->addUnitMapValues(this, QStringList() << singular << plural << symbol << synonyms);
    }
    d->multiplier = multiplier;
    d->singular = singular;
    d->plural = plural;
    d->symbol = symbol;
}

Unit::Unit(QObject* parent, const QString& singular, const QString& plural, const QString& symbol,
           const Complex* complex, const QStringList& synonyms)
: QObject(parent)
, d(new Unit::Private)
{
    UnitCategory* uc = category();
    if (uc) {
        uc->addUnitMapValues(this, QStringList() << singular << plural << symbol << synonyms);
    }
    d->complex = complex;
    d->singular = singular;
    d->plural = plural;
    d->symbol = symbol;
}

Unit::~Unit()
{
    delete d;
}

UnitCategory* Unit::category() const
{
    return dynamic_cast<UnitCategory*>(parent());
}

QString Unit::singular() const
{
    return d->singular;
}

QString Unit::plural() const
{
    return d->plural;
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
    if (value == 1.0) {
        return singular();
    }
    return plural();
}

bool Unit::isValid() const
{
    return !d->singular.isEmpty();
}

}

