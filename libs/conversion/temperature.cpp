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

#include "temperature.h"
#include <KDebug>
#include <KLocale>

Temperature::Temperature(QObject* parent)
: UnitCategory(parent)
{
    setObjectName("temperature");
}

QString Temperature::name() const
{
    return i18n("Temperature");
}

bool Temperature::hasUnit(const QString &unit) const
{
    return (QString("KFC").indexOf(unit) != -1);
}

QStringList Temperature::units() const
{
    return QStringList() << "\xb0""C" << "\xb0""F" << "\xb0""K";
}

Conversion::Value Temperature::convert(const Conversion::Value& value, const QString& toUnit) const
{
    QString to = (toUnit.isEmpty())? "K" : toUnit;
    double temp = value.number().toDouble();

    if (value.unit().contains("C")) {
        temp += 273.15;
    } else if (value.unit().contains("F")){
        temp = ((temp - 32.0) / 1.8) + 273.15;
    }
    if (to.contains("C")) {
        temp -= 273.15;
    } else if (to.contains("F")){
        temp = ((temp - 273.15) * 1.8) + 32.0;
    }
    return Conversion::Value(temp, to);
}
