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
#include <QString>
#include <QStringList>
#include "plasmaconversion_export.h"

namespace Conversion
{

class PLASMACONVERSION_EXPORT Complex
{
public:
    Complex() {};
    virtual ~Complex() {};
    virtual double toDefault(double) const = 0;
    virtual double fromDefault(double) const = 0;
};

class PLASMACONVERSION_EXPORT Unit : public QObject
{
public:
    explicit Unit(QObject* parent, const QString& singular, const QString& plural,
                  const QString& symbol, double multiplier,
                  const QStringList& synonyms = QStringList());
    explicit Unit(QObject* parent, const QString& singular, const QString& plural,
                  const QString& symbol, const Complex* multiplier,
                  const QStringList& synonyms = QStringList());
    virtual ~Unit();
    /**
     * @return translated name for unit.
     **/
    QString singular() const;

    /**
     * @return translated name for unit (plural).
     **/
    QString plural() const;

    /**
     * @return symbol for the unit.
     **/
    QString symbol() const;

    /**
     * @return unit multiplier.
     **/
    double multiplier() const;

    /**
     * Set unit multiplier.
     **/
    void setMultiplier(double multiplier);

protected:
    double toDefault(double value) const;
    double fromDefault(double value) const;

private:
    friend class UnitCategory;
    class Private;
    Private* const d;
};

#define U(n, p, s, m, sy) (new Conversion::Unit(this, n, p, s, m, QStringList() sy))

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
    QString name() const;

    /**
     * Returns default unit.
     *
     * @return default unit.
     **/
    QString defaultUnit() const;

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
     * @return Translated list of units.
     **/
    QStringList units() const;

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
    virtual Value convert(const Value& value, const QString& toUnit = QString());

protected:
    void addSIUnit(const QString& symbol, const QString& single, const QString& plural,
                   uint multiplier = 1, double shift = 1.0);
    void setName(const QString& name);
    void setDefaultUnit(const QString& defaultUnit);
    void addUnitName(const QString& name);
    void addUnitMapValues(Unit* unit, const QStringList& names);

private:
    friend class Unit;
    class Private;
    Private* const d;
};

} // Conversion namespace

#endif
