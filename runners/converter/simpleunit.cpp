/*
 *   Copyright (C) 2007 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "simpleunit.h"
#include <QVariant>
#include <kdebug.h>

SimpleUnit::SimpleUnit()
{
}

bool SimpleUnit::hasUnit(const QString &unit)
{
    return m_units.contains(unit);
}

QString SimpleUnit::convert(const QString &value, const QString &from,
                        const QString &to, QVariant* data)
{
    QString unit;
    *data = value.toDouble() * toDouble(from, &unit) / toDouble(to, &unit);
    return QString("%1 %2").arg(data->toString()).arg(unit);
}

double SimpleUnit::toDouble(const QString &unit, QString *unitString)
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
