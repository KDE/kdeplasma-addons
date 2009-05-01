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

#include "apodprovider.h"

#include <QtCore/QDate>
#include <QtCore/QRegExp>
#include <QtGui/QImage>

#include <KDebug>
#include <kio/job.h>

POTDPROVIDER_EXPORT_PLUGIN( ApodProvider, "ApodProvider", "" )

class ApodProvider::Private
{
  public:
    Private( ApodProvider *parent )
      : mParent( parent )
    {
    }

    void pageRequestFinished( KJob* );
    void imageRequestFinished( KJob* );
    void parsePage();

    ApodProvider *mParent;
    QByteArray mPage;
    QDate mDate;
    QImage mImage;
};

void ApodProvider::Private::pageRequestFinished( KJob *_job )
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit mParent->error( mParent );
	return;
    }
    
    const QString data = QString::fromUtf8( job->data() );
    
    const QString pattern( "<IMG SRC=\"image/*.jpg" );
    QRegExp exp( pattern );
    exp.setPatternSyntax(QRegExp::Wildcard);
    
    int pos = exp.indexIn( data ) + pattern.length();
    
    const QString sub = data.mid( pos, exp.matchedLength() -21);
    kDebug() << "####### sub " << sub << endl; //TODO should return *.jpg
    
    KUrl url( QString( "http://antwrp.gsfc.nasa.gov/apod/image/%1/%2" ).arg(QDate::currentDate().toString( "yyMM" ) ).arg( sub ) );

    KIO::StoredTransferJob *imageJob = KIO::storedGet( url );
    mParent->connect( imageJob, SIGNAL( finished( KJob* ) ), SLOT( imageRequestFinished( KJob* ) ) );
}

void ApodProvider::Private::imageRequestFinished( KJob *_job )
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit mParent->error( mParent );
	return;
    }

    mImage = QImage::fromData( job->data() );
    emit mParent->finished( mParent );
}

ApodProvider::ApodProvider( QObject *parent, const QVariantList &args )
    : PotdProvider( parent, args ), d( new Private( this ) )
{
    const QString type = args[ 0 ].toString();
    if ( type == "Date" )
        d->mDate = args[ 1 ].toDate();
    else
	Q_ASSERT( false && "Invalid type passed to potd provider" );

    KUrl url( QString( "http://antwrp.gsfc.nasa.gov/apod/" ) );
    KIO::StoredTransferJob *job = KIO::storedGet( url );
    connect( job, SIGNAL( finished( KJob *) ), SLOT( pageRequestFinished( KJob* ) ) );
}

ApodProvider::~ApodProvider()
{
    delete d;
}

QImage ApodProvider::image() const
{
    return d->mImage;
}

QString ApodProvider::identifier() const
{
    return QString( "apod:%1" ).arg( d->mDate.toString( Qt::ISODate ));
}

#include "apodprovider.moc"
