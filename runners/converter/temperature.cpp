/*
 *   Copyright (C) 2007 Petri Damstén <damu@iki.fi>
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

#include "temperature.h"
#include <kdebug.h>

bool Temperature::hasUnit(const QString &unit)
{
    return (QString("KFC").indexOf(unit) != -1);
}

QString Temperature::convert(const QString &value, const QString &from,
                        const QString &to, QVariant* data)
{
    QString toUnit = (to.isEmpty())? "K" :to;
    double temp = value.toDouble();

    if (from == "C") {
        temp += 273.15;
    } else if (from == "F"){
        temp = ((temp - 32.0) / 1.8) + 273.15;
    }
    if (toUnit == "C") {
        temp -= 273.15;
    } else if (toUnit == "F"){
        temp = ((temp - 273.15) * 1.8) + 32.0;
    }
    return QString("%1 %2").arg(temp).arg(toUnit);
}
