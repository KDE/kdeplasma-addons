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

#ifndef UNITCATEGORY_H
#define UNITCATEGORY_H

#include <QString>
#include <QVariant>
#include <QStringList>
#include "plasmaconverter_export.h"

class PLASMACONVERTER_EXPORT Value
{
public:
    Value();
    Value(const QVariant& n, const QString& u);

    /**
     * Check if value is valid.
     *
     * @return True if value is valid
     **/
    bool isValid();

    /**
     * Convert value to a string
     *
     * @return value as string
     **/
    QString toString();

    /**
     * Number part of the value
     **/
    QVariant number;

    /**
     * Unit part of the value
     **/
    QString unit;
};

class KDE_EXPORT UnitCategory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name)
public:
    explicit UnitCategory(QObject* parent = 0);
    virtual ~UnitCategory();

    /**
     * Returns name for the unit category.
     *
     * @return Translated name for category.
     **/
    virtual QString name() = 0;

    /**
     * Check if unit category has a unit.
     *
     * @return True if unit is found
     **/
    virtual bool hasUnit(const QString &unit) = 0;

    /**
     * Return units in this category.
     *
     * @return list of units.
     **/
    virtual QStringList units() = 0;

    /**
     * Convert value to another unit.
     *
     * @param value value to convert
     * @param toUnit unit to convert to. If empty default unit is used.
     * @return converted value
     **/
    virtual Value convert(const Value& value, const QString& toUnit = QString()) = 0;
};

#endif
