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
