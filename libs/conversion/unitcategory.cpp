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

#include "unitcategory.h"
#include <KLocale>
#include <KDebug>

namespace Conversion
{

class UnitCategory::Private
{
public:
    Private() :
        defaultUnit(0)
    {
    };

    ~Private()
    {
    };
    QString name;
    Unit* defaultUnit;
    QMap<QString, Unit*> unitMap;
    QMap<int, Unit*> idMap;
    QString description;
    KUrl url;
};

UnitCategory::UnitCategory(QObject* parent)
: QObject(parent)
, d(new UnitCategory::Private)
{
}

UnitCategory::~UnitCategory()
{
    delete d;
}

QList<Unit*> UnitCategory::units() const
{
    return findChildren<Unit*>();
}

QStringList UnitCategory::allUnits() const
{
    return d->unitMap.keys();
}

bool UnitCategory::hasUnit(const QString &unit) const
{
    return d->unitMap.keys().contains(unit);
}

Conversion::Value UnitCategory::convert(const Conversion::Value& value, const QString& toUnit)
{
    if ((toUnit.isEmpty() || d->unitMap.keys().contains(toUnit)) && value.unit()->isValid()) {
        const Unit* to = toUnit.isEmpty() ? defaultUnit() : d->unitMap[toUnit];
        return convert(value, to);
    }
    return Value();
}

Conversion::Value UnitCategory::convert(const Conversion::Value& value, int toUnit)
{
    if (d->idMap.keys().contains(toUnit) && value.unit()->isValid()) {
        return convert(value, d->idMap[toUnit]);
    }
    return Value();
}

Conversion::Value UnitCategory::convert(const Conversion::Value& value, const Unit* toUnit)
{
    if (toUnit) {
        double v = toUnit->fromDefault(value.unit()->toDefault(value.number()));
        return Conversion::Value(v, toUnit);
    }
    return Value();
}

void UnitCategory::addUnitMapValues(Unit* unit, const QString& names)
{
    QStringList list = names.split(';');
    foreach (const QString& name, list) {
        d->unitMap[name] = unit;
    }
}

void UnitCategory::addIdMapValue(Unit* unit, int id)
{
    d->idMap[id] = unit;
}

Unit* UnitCategory::unit(const QString& s) const
{
    if (d->unitMap.keys().contains(s)) {
        return d->unitMap[s];
    }
    return 0;
}

QString UnitCategory::name() const
{
    return d->name;
}

void UnitCategory::setName(const QString& name)
{
    d->name = name;
}

void UnitCategory::setDefaultUnit(Unit* defaultUnit)
{
    d->defaultUnit = defaultUnit;
}

Unit* UnitCategory::defaultUnit() const
{
    return d->defaultUnit;
}

QString UnitCategory::description() const
{
    return d->description;
}

void UnitCategory::setDescription(const QString& description)
{
    d->description = description;
}

KUrl UnitCategory::url() const
{
    return d->url;
}

void UnitCategory::setUrl(const KUrl& url)
{
    d->url = url;
}

}

#include "unitcategory.moc"

