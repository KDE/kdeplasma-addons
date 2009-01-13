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

#ifndef CONVERSION_VALUE_H
#define CONVERSION_VALUE_H

#include <QString>
#include <QVariant>
#include "plasmaconversion_export.h"

namespace Conversion
{

class PLASMACONVERSION_EXPORT Value
{
public:
    Value();
    Value(const QVariant& n, const QString& u);
    ~Value();
    /**
     * Check if value is valid.
     *
     * @return True if value is valid
     **/
    bool isValid() const;

    /**
     * Convert value to a string
     *
     * @return value as string
     **/
    QString toString() const;

    /**
     * Number part of the value
     **/
    QVariant number() const;

    /**
     * Unit part of the value
     **/
    QString unit() const;

    /**
     * Set value description
     **/
    void setDescription(const QString& desc);

    /**
     * @return value description
     **/
    QString description() const;

private:
    class Private;
    Private* const d;
};

} // Conversion namespace

#endif
