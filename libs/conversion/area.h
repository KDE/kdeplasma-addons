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

#ifndef AREA_H
#define AREA_H

#include "simpleunit.h"

class Area : public SimpleUnit
{
public:
    Area(QObject* parent = 0);
    virtual bool hasUnit(const QString &unit) const;
    virtual QString name() const;

protected:
    QString replace(const QString &unit) const;
    virtual double toDefault(double value, const QString &unit) const;
    virtual double fromDefault(double value, const QString &unit) const;
};

#endif
