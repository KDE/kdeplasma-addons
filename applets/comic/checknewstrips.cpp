/***************************************************************************
 *   Copyright (C) 2011 Matthias Fuchs <mat69@gmx.net>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "checknewstrips.h"

#include <QtCore/QTimer>

CheckNewStrips::CheckNewStrips( const QStringList &identifiers, Plasma::DataEngine *engine, int minutes, QObject *parent)
  : QObject( parent ),
    mMinutes( minutes ),
    mIndex( 0 ),
    mEngine( engine ),
    mIdentifiers( identifiers )
{
    QTimer *timer = new QTimer( this );
    timer->setInterval( minutes * 60 * 1000 );
    connect( timer, SIGNAL(timeout()), this, SLOT(start()) );
    timer->start();

    //start at once, that way the user does not have to wait for minutes to get the initial result
    start();
}

void CheckNewStrips::dataUpdated( const QString &source, const Plasma::DataEngine::Data &data )
{
    QString lastIdentifierSuffix;

    if ( !data[ "Error" ].toBool() ) {
        lastIdentifierSuffix = data[ "Identifier" ].toString();
        lastIdentifierSuffix.remove( source );
    }

    mEngine->disconnectSource( source, this );

    if ( !lastIdentifierSuffix.isEmpty() ) {
        QString temp = source;
        temp.remove( ':' );
        emit lastStrip( mIndex, temp, lastIdentifierSuffix );
    }
    ++mIndex;

    if ( mIndex < mIdentifiers.count() ) {
        const QString newSource = mIdentifiers[mIndex] + ':';
        mEngine->connectSource( newSource, this );
        mEngine->query( newSource );
    } else {
        mIndex = 0;
    }
}

void CheckNewStrips::start()
{
    //already running, do nothing
    if ( mIndex ) {
        return;
    }

    if ( mIndex < mIdentifiers.count() ) {
        const QString newSource = mIdentifiers[mIndex] + ':';
        mEngine->connectSource( newSource, this );
        mEngine->query( newSource );
    }
}
