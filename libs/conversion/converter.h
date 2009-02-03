/*
 *   Copyright (C) 2008-2009 Petri Damstén <damu@iki.fi>
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

#ifndef CONVERSION_CONVERTER_H
#define CONVERSION_CONVERTER_H

#include "value.h"
#include "plasmaconversion_export.h"
#include <QtCore/QVariant>

namespace Conversion
{

class UnitCategory;

class PLASMACONVERSION_EXPORT Converter : public QObject
{
    Q_OBJECT
public:
    /**
     * Singleton accessor
     **/
    static Converter* self();

    /**
     * Convert value to another unit.
     *
     * @param value value to convert
     * @param toUnit unit to convert to. If empty default unit is used.
     * @return converted value
     **/
    Value convert(const Value& value, const QString& toUnit = QString()) const;

    /**
     * Find unit category for unit.
     *
     * @param unit unit to find category for.
     * @return unit category for unit
     **/
    UnitCategory* categoryForUnit(const QString& unit) const;

    /**
     * Find unit for string unit.
     *
     * @param unit unit string to find unit for.
     * @return unit for string unit
     **/
    Unit* unit(const QString& unit) const;

    /**
     * Find unit category.
     *
     * @param category name of the category to find (length, area, mass, etc.).
     * @return unit category named category or 0.
     **/
    UnitCategory* category(const QString& category) const;

    /**
     * Returns a list of all unit categories.
     *
     * @return list of unit categories.
     **/
    QList<UnitCategory*> categories() const;

private:
    friend class ConverterSingleton;

    explicit Converter(QObject* parent = 0);
    ~Converter();

    class Private;
    Private* const d;
};

} // Conversion namespace

#endif
