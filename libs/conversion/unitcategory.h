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

#ifndef CONVERSION_UNITCATEGORY_H
#define CONVERSION_UNITCATEGORY_H

#include "value.h"
#include "unit.h"
#include <QString>
#include <QStringList>
#include <KUrl>
#include "plasmaconversion_export.h"

namespace Conversion
{

class PLASMACONVERSION_EXPORT UnitCategory : public QObject
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
    QString name() const;

    /**
     * Set value description
     **/

    /**
     * @return unit category description
     **/
    QString description() const;

    /**
     * @return unit category url for description
     **/
    KUrl url() const;

    /**
     * Returns default unit.
     *
     * @return default unit.
     **/
    Unit* defaultUnit() const;

    /**
     * Check if unit category has a unit.
     *
     * @return True if unit is found
     **/
    bool hasUnit(const QString &unit) const;

    /**
     * Return unit for string.
     *
     * @return Pointer to unit class.
     **/
    Unit* unit(const QString& s) const;

    /**
     * Return units in this category.
     *
     * @return list of units.
     **/
    QList<Unit*> units() const;

    /**
     * Return all unit names, short names and unit synonyms in this category.
     *
     * @return list of units.
     **/
    QStringList allUnits() const;

    /**
     * Convert value to another unit.
     *
     * @param value value to convert
     * @param toUnit unit to convert to. If empty default unit is used.
     * @return converted value
     **/
    Value convert(const Value& value, const QString& toUnit = QString());
    virtual Value convert(const Value& value, const Unit* toUnit);

protected:
    void addSIUnit(const QString& symbol, const QString& single, const QString& plural,
                   uint multiplier = 1, double shift = 1.0);
    void setName(const QString& name);
    void setDefaultUnit(Unit* defaultUnit);
    void addUnitName(const QString& name);
    void addUnitMapValues(Unit* unit, const QStringList& names);
    void setDescription(const QString& desc);
    void setUrl(const KUrl& url);

private:
    friend class Unit;
    class Private;
    Private* const d;
};

} // Conversion namespace

#endif
