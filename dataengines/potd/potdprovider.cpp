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

    Private()
        : isFixedDate(false)
    {
    }

    QDate date;
    bool isFixedDate;
};

PotdProvider::PotdProvider( QObject *parent, const QVariantList &args )
    : QObject( parent ),
      d(new Private)
{
    if ( args.count() > 0 ) {
        if ( args[ 0 ].canConvert( QVariant::Date ) ) {
            d->date = args[ 0 ].toDate();
            d->isFixedDate = true;
        }
    }
}

PotdProvider::~PotdProvider()
{
}

QDate PotdProvider::date() const
{
    return d->isFixedDate ? d->date : QDate::currentDate();
}

bool PotdProvider::isFixedDate() const
{
    return d->isFixedDate;
}

#include "potdprovider.moc"
