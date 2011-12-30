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

class PotdProvider::Private
{
public:
    QString name;
    QDate date;
};

PotdProvider::PotdProvider( QObject *parent, const QVariantList &args )
    : QObject( parent ),
      d(new Private)
{
    if ( args.count() > 0 ) {
        d->name = args[ 0 ].toString();

        if ( args.count() > 1 && args[ 1 ].canConvert( QVariant::Date ) ) {
            d->date = args[ 1 ].toDate();
        }
    } else {
        d->name = "Unknown";
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
    if (isFixedDate()) {
        return QString( QLatin1String( "%1:%2" ) ).arg( d->name, d->date.toString( Qt::ISODate ));
    }

    return d->name;
}


#include "potdprovider.moc"
