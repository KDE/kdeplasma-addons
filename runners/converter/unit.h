/*
 *   Copyright (C) 2007 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef UNIT_H
#define UNIT_H

class QString;
class QVariant;

class Unit
{
public:
    Unit() {};
    virtual ~Unit() {};

    virtual bool hasUnit(const QString &unit) = 0;
    virtual QString convert(const QString &value, const QString &from,
                            const QString &to, QVariant* data) = 0;
};

#endif
