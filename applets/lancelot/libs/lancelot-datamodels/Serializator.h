/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOT_SERIALIZATOR_H
#define LANCELOT_SERIALIZATOR_H

#include <lancelot/lancelot_export.h>

#include <QMap>

namespace Lancelot {
namespace Models {

/**
 * Utility class for (de)serializing string to string maps
 */
class LANCELOT_EXPORT Serializator {
public:
    /**
     * @param data data to be serialized
     * @returns serialized string
     */
    static QString serialize(const QMap < QString , QString > data);

    /**
     * @param data to be deserialized
     * @returns map
     */
    static QMap < QString , QString > deserialize(const QString & data);
};

} // namespace Models
} // namespace Lancelot

#endif /* LANCELOT_SERIALIZATOR_H */
