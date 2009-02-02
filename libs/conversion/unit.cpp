/*
 * Copyright (C) 2007,2008 Petri Damstén <damu@iki.fi>
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

#include "unit.h"
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

Unit::Unit(QObject* parent, const QString& singular, const QString& plural, const QString& symbol,
           double multiplier, const QStringList& synonyms)
: QObject(parent)
, d(new Unit::Private)
{
    UnitCategory* uc = dynamic_cast<UnitCategory*>(parent);
    if (uc) {
        uc->addUnitName(singular);
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
    UnitCategory* uc = dynamic_cast<UnitCategory*>(parent);
    if (uc) {
        uc->addUnitName(singular);
        uc->addUnitMapValues(this, QStringList() << singular << plural << symbol << synonyms);
    }
    d->complex = complex;
    d->singular = singular;
    d->plural = plural;
    d->symbol = symbol;
}

Unit::~Unit()
{
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
    QString defaultUnit;
    QStringList unitNames;
    QMap<QString, Unit*> unitMap;

};

UnitCategory::UnitCategory(QObject* parent)
: QObject(parent)
, d(new UnitCategory::Private)
{
}

UnitCategory::~UnitCategory()
{
}

QStringList UnitCategory::units() const
{
    return d->unitNames;
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
    if ((toUnit.isEmpty() || d->unitMap.keys().contains(toUnit)) && d->unitMap.keys().contains(value.unit())) {
        const Unit* to = d->unitMap[toUnit.isEmpty() ? defaultUnit() : toUnit];
        const Unit* from = d->unitMap[value.unit()];
        double v = to->fromDefault(from->toDefault(value.number().toDouble()));
        if (v == 1.0) {
            return Conversion::Value(v, to->singular());
        } else {
            return Conversion::Value(v, to->plural());
        }
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
            kDebug() << j << d;
        }
        kDebug() << prefix + prefixes[i] + single << list << d;
        new Unit(this, prefix + prefixes[i] + single, prefix + prefixes[i] + plural,
                 symbols[i] + symbol, d, list);
    }
}

void UnitCategory::addUnitName(const QString& name)
{
    d->unitNames << name;
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

void UnitCategory::setDefaultUnit(const QString& defaultUnit)
{
    d->defaultUnit = defaultUnit;
}

QString UnitCategory::defaultUnit() const
{
    return d->defaultUnit;
}

}

#include "unit.moc"

