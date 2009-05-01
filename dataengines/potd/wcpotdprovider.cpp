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

#include "wcpotdprovider.h"

#include <QtCore/QDate>
#include <QtCore/QRegExp>
#include <QtGui/QImage>

#include <KDebug>
#include <kio/job.h>

POTDPROVIDER_EXPORT_PLUGIN( WcpotdProvider, "WcpotdProvider", "" )

class WcpotdProvider::Private
{
  public:
    Private( WcpotdProvider *parent )
      : mParent( parent )
    {
    }

    void pageRequestFinished( KJob* );
    void imageRequestFinished( KJob* );
    void parsePage();

    WcpotdProvider *mParent;
    QByteArray mPage;
    QDate mDate;
    QImage mImage;
};

void WcpotdProvider::Private::pageRequestFinished( KJob *_job )
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit mParent->error( mParent );
	return;
    }

    const QString data = QString::fromUtf8( job->data() );
    //get image URL
    const QString pattern( "<a href=\"http://commons.wikimedia.org/wiki/Image:*.jpg" );
    QRegExp exp( pattern );
    exp.setPatternSyntax(QRegExp::Wildcard);
    int pos = exp.indexIn( data );
    //38 is for <img src=\"http://commons.wikimedia.org
    const QString sub = data.mid( pos+49, pattern.length()-14);//FIXME check if this really works!!!
    KUrl picUrl( QString( "http://toolserver.org/tsthumb/tsthumb?f=%1&amp;domain=commons.wikimedia.org&amp;w=800" ).arg( sub ) );
    KIO::StoredTransferJob *imageJob = KIO::storedGet( picUrl );
    mParent->connect( imageJob, SIGNAL( finished( KJob *) ), SLOT( imageRequestFinished( KJob* ) ) );
}

void WcpotdProvider::Private::imageRequestFinished( KJob *_job )
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit mParent->error( mParent );
	return;
    }
    QByteArray data = job->data();
    mImage = QImage::fromData( data );
    emit mParent->finished( mParent );
}

WcpotdProvider::WcpotdProvider( QObject *parent, const QVariantList &args )
    : PotdProvider( parent, args ), d( new Private( this ) )
{
    const QString type = args[ 0 ].toString();
    if ( type == "Date" )
        d->mDate = args[ 1 ].toDate();
    else
	Q_ASSERT( false && "Invalid type passed to potd provider" );

    KUrl url( "http://toolserver.org/~daniel/potd/commons/potd-800x600.snippet" );
    KIO::StoredTransferJob *job = KIO::storedGet( url );
    connect( job, SIGNAL( finished( KJob *) ), SLOT( pageRequestFinished( KJob* ) ) );
}

WcpotdProvider::~WcpotdProvider()
{
    delete d;
}

QImage WcpotdProvider::image() const
{
    return d->mImage;
}

QString WcpotdProvider::identifier() const
{
    return QString( "wcpotd:%1" ).arg( d->mDate.toString( Qt::ISODate ));
}

#include "wcpotdprovider.moc"
