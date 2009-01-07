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

#include "simpleunit.h"
#include <QVariant>
#include <KDebug>
#include <KLocale>

SimpleUnit::SimpleUnit(QObject* parent)
: UnitCategory(parent)
{
}

QStringList SimpleUnit::units() const
{
    QStringList result;

    foreach (const QString& unit, m_units.keys()) {
        // Only return basic units
        if  (m_units[unit].type() == QVariant::Double) {
            result << unit;
        }
    }
    return result;
}

bool SimpleUnit::hasUnit(const QString &unit) const
{
    return m_units.contains(unit);
}

Conversion::Value SimpleUnit::convert(const Conversion::Value& value, const QString& to) const
{
    QString unit;
    QVariant data = value.number().toDouble() * toDouble(value.unit(), &unit) / toDouble(to, &unit);
    return Conversion::Value(data, unit);
}

double SimpleUnit::toDouble(const QString &unit, QString *unitString) const
{
    if (unit.isEmpty()) {
        *unitString = m_default;
    } else {
        *unitString = unit;
    }
    QVariant multiplier = m_units[*unitString];
    if (multiplier.type() == QVariant::Double) {
        return multiplier.toDouble();
    } else {
        *unitString = multiplier.toString();
        return m_units[*unitString].toDouble();
    }
}

void SimpleUnit::addSIUnit(const QString& unit, const QString& single, const QString& plural)
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

    for (int i = 0; i < prefixes.count(); ++i) {
            m_units[prefixes[i] + single] = symbols[i] + unit;
            m_units[prefixes[i] + plural] = symbols[i] + unit;
            m_units[symbols[i] + unit]    = decimals[i];
    }
}
