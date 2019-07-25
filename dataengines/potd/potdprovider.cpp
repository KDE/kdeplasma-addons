/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify  
 *   it under the terms of the GNU General Public License as published by  
 *   the Free Software Foundation; either version 2 of the License, or     
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

#include "potdprovider.h"

// Qt
#include <QDate>

class PotdProviderPrivate
{
public:
    QString name;
    QDate date;
    QString identifier;
};

PotdProvider::PotdProvider( QObject *parent, const QVariantList &args )
    : QObject( parent ),
      d(new PotdProviderPrivate)
{
    if ( args.count() > 0 ) {
        d->name = args[ 0 ].toString();
        
        d->identifier = d->name;

        if ( args.count() > 1 ) {
            for (int i = 1; i < args.count(); i++) {
                d->identifier += QStringLiteral(":") + args[i].toString();
                QDate date = QDate::fromString(args[ i ].toString(), Qt::ISODate);
                if (date.isValid()) {
                    d->date = date;
                }
            }
        }
    } else {
        d->name = QStringLiteral("Unknown");
        d->identifier = d->name;
    }
}

PotdProvider::~PotdProvider()
{
}

QString PotdProvider::name() const
{
    return d->name;
}

QDate PotdProvider::date() const
{
    return d->date.isNull() ? QDate::currentDate() : d->date;
}

bool PotdProvider::isFixedDate() const
{
    return !d->date.isNull();
}

QString PotdProvider::identifier() const
{
    return d->identifier;
}


