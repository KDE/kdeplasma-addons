/*
 *   SPDX-FileCopyrightText: 2011 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "checknewstrips.h"

#include <QTimer>

CheckNewStrips::CheckNewStrips( const QStringList &identifiers, Plasma::DataEngine *engine, int minutes, QObject *parent)
  : QObject( parent ),
    mMinutes( minutes ),
    mIndex( 0 ),
    mEngine( engine ),
    mIdentifiers( identifiers )
{
    QTimer *timer = new QTimer( this );
    timer->setInterval( minutes * 60 * 1000 );
    connect( timer, &QTimer::timeout, this, &CheckNewStrips::start );
    timer->start();

    //start at once, that way the user does not have to wait for minutes to get the initial result
    start();
}

void CheckNewStrips::dataUpdated( const QString &source, const Plasma::DataEngine::Data &data )
{
    QString lastIdentifierSuffix;

    if (!data[QStringLiteral("Error")].toBool()) {
        lastIdentifierSuffix = data[QStringLiteral("Identifier")].toString();
        lastIdentifierSuffix.remove( source );
    }

    mEngine->disconnectSource( source, this );

    if ( !lastIdentifierSuffix.isEmpty() ) {
        QString temp = source;
        temp.remove(QLatin1Char(':'));
        emit lastStrip( mIndex, temp, lastIdentifierSuffix );
    }
    ++mIndex;

    if ( mIndex < mIdentifiers.count() ) {
        const QString newSource = mIdentifiers[mIndex] + QLatin1Char(':');
        mEngine->connectSource( newSource, this );
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
        const QString newSource = mIdentifiers[mIndex] + QLatin1Char(':');
        mEngine->connectSource( newSource, this );
    }
}
