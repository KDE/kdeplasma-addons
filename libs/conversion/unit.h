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

#ifndef CONVERSION_UNITCATEGORY_H
#define CONVERSION_UNITCATEGORY_H

#include "value.h"
#include <QString>
#include <QVariant>
#include <QStringList>
#include "plasmaconversion_export.h"

namespace Conversion
{

class PLASMACONVERSION_EXPORT UnitCategory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QStringList units READ units)
public:
    explicit UnitCategory(QObject* parent = 0);
    virtual ~UnitCategory();

    /**
     * Returns name for the unit category.
     *
     * @return Translated name for category.
     **/
    virtual QString name() const = 0;

    /**
     * Check if unit category has a unit.
     *
     * @return True if unit is found
     **/
    virtual bool hasUnit(const QString &unit) const = 0;

    /**
     * Return units in this category.
     *
     * @return list of units.
     **/
    virtual QStringList units() const = 0;

    /**
     * Convert value to another unit.
     *
     * @param value value to convert
     * @param toUnit unit to convert to. If empty default unit is used.
     * @return converted value
     **/
    virtual Value convert(const Value& value, const QString& toUnit = QString()) const = 0;
};

} // Conversion namespace

#endif
