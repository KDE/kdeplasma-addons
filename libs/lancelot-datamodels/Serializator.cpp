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

#include "Serializator.h"

#include <QMap>
#include <QMapIterator>
#include <QString>
#include <QStringList>
#include <QUrl>

namespace Lancelot {
namespace Models {

QString Serializator::serialize(const QMap < QString , QString > data)
{
    QString result;
    QMapIterator < QString, QString > i(data);
    while (i.hasNext()) {
        i.next();

        if (!result.isEmpty()) {
            result += '&';
        }

        result +=
            QUrl::toPercentEncoding(i.key()) + '=' +
            QUrl::toPercentEncoding(i.value());
    }
    return result;
}

QMap < QString , QString > Serializator::deserialize(const QString & data)
{
    QMap < QString , QString > result;

    QStringList items = data.split('&');
    foreach (const QString & item, items) {
        QStringList broken = item.split('=');
        if (broken.size() != 2) {
            continue;
        }
        result[QUrl::fromPercentEncoding(broken.at(0).toAscii())] =
            QUrl::fromPercentEncoding(broken.at(1).toAscii());
    }

    return result;
}

} // namespace Models
} // namespace Lancelot
