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
    Private()
    {
    };

    ~Private()
    {
    };
    QString name;
    Unit* defaultUnit;
    QMap<QString, Unit*> unitMap;
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

Conversion::Value UnitCategory::convert(const Conversion::Value& value, const Unit* toUnit)
{
    if (toUnit) {
        double v = toUnit->fromDefault(value.unit()->toDefault(value.number()));
        return Conversion::Value(v, toUnit);
    }
    return Value();
}

void UnitCategory::addSIUnit(const QString& symbol, const QString& single, const QString& plural,
                             uint multiplier, double shift)
{
    static const QStringList prefixes = QStringList() <<
            i18n("yotta") << i18n("zetta") << i18n("exa") << i18n("peta") << i18n("tera") <<
            i18n("giga") << i18n("mega") << i18n("kilo") << i18n("hecto") << i18n("deca") <<
            "" << i18n("deci") << i18n("centi") << i18n("milli") << i18n("micro") <<
            i18n("nano") << i18n("pico") << i18n("femto") << i18n("atto") << i18n("zepto") <<
            i18n("yocto");
    static const QStringList symbols = QStringList() <<
            "Y" << "Z" << "E" << "P" << "T" << "G" << "M" << "k" << "h" << "da" << "" << "d" <<
            "c" << "m" << "\xb5" << "n" << "p" << "f" << "a" << "z" << "y";
    static const QList<double> decimals = QList<double>() <<
            1.0E+24 << 1.0E+21 << 1.0E+18 << 1.0E+15 << 1.0E+12 << 1.0E+9 << 1.0E+6 << 1.0E+3 <<
            1.0E+2 << 1.0E+1 << 1.0 << 1.0E-1 << 1.0E-2 << 1.0E-3 << 1.0E-6 << 1.0E-9 <<
            1.0E-12 << 1.0E-15 << 1.0E-18 << 1.0E-21 << 1.0E-24;

    QString prefix;
    QStringList suffixes;
    switch (multiplier) {
        case 2:
            prefix = i18n("square ");
            suffixes << "/-2" << "^2" << "2";
            break;
        case 3:
            prefix = i18n("cubic ");
            suffixes << "/-3" << "^3" << "3";
            break;
    }
    for (int i = 0; i < prefixes.count(); ++i) {
        QStringList list;
        if (symbols[i] == "\xb5") {
            list << 'u' + symbol;
        }
        foreach (const QString& suffix, suffixes) {
            list << symbols[i] + symbol.left(symbol.length() - 1) + suffix;
        }
        double d = decimals[i] / shift;
        for (uint j = 1; j < multiplier; ++j) {
            d *= decimals[i];
        }
        Unit* u = new Unit(this, prefix + prefixes[i] + single, prefix + prefixes[i] + plural,
                           symbols[i] + symbol, d, list);
        if (prefixes[i].isEmpty()) {
            setDefaultUnit(u);
        }
    }
}

void UnitCategory::addUnitMapValues(Unit* unit, const QStringList& names)
{
    foreach (const QString& name, names) {
        d->unitMap[name] = unit;
    }
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

