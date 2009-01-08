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

#ifndef SIMPLEUNIT_H
#define SIMPLEUNIT_H

#include "value.h"
#include "unit.h"
#include <QHash>
#include <KDebug>

class Complex
{
public:
    Complex() {};
    virtual ~Complex() {};
    virtual double toDefault(double) const { return 0.0; };
    virtual double fromDefault(double) const { return 0.0; };
};
Q_DECLARE_METATYPE(Complex*)

class SimpleUnit : public Conversion::UnitCategory
{
public:
    SimpleUnit(QObject* parent = 0);
    virtual ~SimpleUnit();

    virtual QStringList units() const;
    virtual bool hasUnit(const QString &unit) const;
    virtual Conversion::Value convert(const Conversion::Value& value, const QString& to) const;

protected:
    QHash<QString, QVariant> m_units;
    QString m_default;

    void addSIUnit(const QString& unit, const QString& single, const QString& plural);
    QString checkUnit(const QString& unit) const;
    virtual double toDefault(double value, const QString &unit) const;
    virtual double fromDefault(double value, const QString &unit) const;
};

#endif
