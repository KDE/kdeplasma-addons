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

#ifndef CONVERSION_UNIT_H
#define CONVERSION_UNIT_H

#include <QString>
#include <QStringList>
#include "plasmaconversion_export.h"

namespace Conversion
{

class UnitCategory;

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
    Unit(QObject* parent = 0);
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
     * @return singular or plural based on value.
     **/
    QString toString(double value) const;

    /**
     * @return unit multiplier.
     **/
    double multiplier() const;

    /**
     * Set unit multiplier.
     **/
    void setMultiplier(double multiplier);

    /**
     * @return unit category.
     **/
    UnitCategory* category() const;

    /**
     * @return if unit is valid.
     **/
    bool isValid() const;

protected:
    double toDefault(double value) const;
    double fromDefault(double value) const;

private:
    friend class UnitCategory;
    class Private;
    Private* const d;
};

#define U(n, p, s, m, sy) (new Conversion::Unit(this, n, p, s, m, QStringList() sy))

} // Conversion namespace

#endif
