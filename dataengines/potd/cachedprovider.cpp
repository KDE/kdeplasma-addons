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

#include "cachedprovider.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtGui/QImage>

#include <kstandarddirs.h>

QString CachedProvider::identifierToPath( const QString &identifier )
{
    const QString dataDir = KStandardDirs::locateLocal( "data", QLatin1String("plasma_engine_podt/") );
    return QString( dataDir + identifier );
}


CachedProvider::CachedProvider( const QString &identifier, QObject *parent )
    : PotdProvider( parent ), mIdentifier( identifier )
{
    QTimer::singleShot( 0, this, SLOT(triggerFinished()) );
}

CachedProvider::~CachedProvider()
{
}

QImage CachedProvider::image() const
{
    if ( !QFile::exists( identifierToPath( mIdentifier ) ) )
        return QImage();

    QImage img;
    img.load( identifierToPath( mIdentifier ), "PNG" );

    return img;
}

QString CachedProvider::identifier() const
{
    return mIdentifier;
}

void CachedProvider::triggerFinished()
{
    emit finished( this );
}

bool CachedProvider::isCached( const QString &identifier )
{
    const QString path = identifierToPath( identifier );
    if (!QFile::exists( path ) ) {
        return false;
    }

    if (!identifier.contains( ':' ) ) {
        // no date in the identifier, so it's a daily; check to see ifthe modification time is today
        QFileInfo info( path );
        if ( info.lastModified().daysTo( QDateTime::currentDateTime() ) > 1 ) {
            return false;
        }
    }

    return true;
}

bool CachedProvider::storeInCache( const QString &identifier, const QImage &potd )
{
    return potd.save( identifierToPath( identifier ), "PNG" );
}

#include "cachedprovider.moc"

