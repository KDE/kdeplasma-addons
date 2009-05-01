/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *   Copyright  2008 by Anne-Marie Mahfouf <annma@kde.org>                 
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

#include "oseiprovider.h"

#include <QtCore/QDate>
#include <QtCore/QRegExp>
#include <QtGui/QImage>

#include <KDebug>
#include <kio/job.h>

POTDPROVIDER_EXPORT_PLUGIN( OseiProvider, "OseiProvider", "" )

class OseiProvider::Private
{
  public:
    Private( OseiProvider *parent )
      : mParent( parent )
    {
    }

    void pageRequestFinished( KJob* );
    void imageRequestFinished( KJob* );
    void parsePage();

    OseiProvider *mParent;
    QByteArray mPage;
    QDate mDate;
    QImage mImage;
};

void OseiProvider::Private::pageRequestFinished( KJob *_job )
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit mParent->error( mParent );
	return;
    }
    // TODO check if this location is always the one for the picture with the same name
    KUrl url( QString( "http://www.osei.noaa.gov/IOD/OSEIiod.jpg" ) );
    KIO::StoredTransferJob *imageJob = KIO::storedGet( url );
    mParent->connect( imageJob, SIGNAL( finished( KJob* ) ), SLOT( imageRequestFinished( KJob* ) ) );
}

void OseiProvider::Private::imageRequestFinished( KJob *_job )
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit mParent->error( mParent );
	return;
    }

    mImage = QImage::fromData( job->data() );
    emit mParent->finished( mParent );
}

OseiProvider::OseiProvider( QObject *parent, const QVariantList &args )
    : PotdProvider( parent, args ), d( new Private( this ) )
{
    const QString type = args[ 0 ].toString();
    if ( type == "Date" )
        d->mDate = args[ 1 ].toDate();
    else
	Q_ASSERT( false && "Invalid type passed to potd provider" );

    KUrl url( QString( "http://www.osei.noaa.gov/OSEIiod.html" ) );
    KIO::StoredTransferJob *job = KIO::storedGet( url );
    connect( job, SIGNAL( finished( KJob *) ), SLOT( pageRequestFinished( KJob* ) ) );
}

OseiProvider::~OseiProvider()
{
    delete d;
}

QImage OseiProvider::image() const
{
    return d->mImage;
}

QString OseiProvider::identifier() const
{
    return QString( "osei:%1" ).arg( d->mDate.toString( Qt::ISODate ));
}

#include "oseiprovider.moc"
