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

#include "epodprovider.h"

#include <QtCore/QDate>
#include <QtCore/QRegExp>
#include <QtGui/QImage>

#include <KDebug>
#include <kio/job.h>

POTDPROVIDER_EXPORT_PLUGIN( EpodProvider, "EpodProvider", "" )

class EpodProvider::Private
{
  public:
    Private( EpodProvider *parent )
        : mParent( parent )
    {
    }

    void pageRequestFinished(KJob*);
    void imageRequestFinished(KJob*);
    void parsePage();

    EpodProvider *mParent;
    QByteArray mPage;
    QDate mDate;
    QImage mImage;
};

void EpodProvider::Private::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if ( job->error() ) {
	emit mParent->error( mParent );
	return;
    }
    
    const QString data = QString::fromUtf8( job->data() );
    kDebug() << "####### data " << data << endl;
    
    // TODO extract the picture name "image/*.jpg" better from data!!!
    const QString pattern( "<IMG SRC= \"archive/images/*.jpg" );
    QRegExp exp( pattern );
    exp.setPatternSyntax(QRegExp::Wildcard);
    
    int pos = exp.indexIn( data ) + pattern.length();
    kDebug() << "length " << exp.matchedLength() << endl;
    const QString sub = data.mid( pos, exp.matchedLength() -31);
    kDebug() << "####### sub " << sub << endl; //TODO should return *.jpg
    
    KUrl url( QString( "http://epod.usra.edu/archive/images/%1" ).arg( sub ) );
    kDebug() << "####### URL " << url.path() << endl; //should return URL for the pic

    KIO::StoredTransferJob *imageJob = KIO::storedGet( url );
    QObject::connect(imageJob, SIGNAL( finished( KJob* )), mParent, SLOT( imageRequestFinished( KJob* ) ) );
}

void EpodProvider::Private::imageRequestFinished( KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if ( job->error() ) {
	emit mParent->error( mParent );
	return;
    }

    mImage = QImage::fromData( job->data() );
    emit mParent->finished( mParent );
}

EpodProvider::EpodProvider( QObject *parent, const QVariantList &args )
    : PotdProvider( parent, args ), d( new Private( this ) )
{
    const QString type = args[ 0 ].toString();
    if ( type == "Date" )
        d->mDate = args[ 1 ].toDate();
    else
	Q_ASSERT( false && "Invalid type passed to potd provider" );

    KUrl url( QString( "http://epod.usra.edu/" ) );
    KIO::StoredTransferJob *job = KIO::storedGet( url );

    connect( job, SIGNAL( finished( KJob* ) ), SLOT( pageRequestFinished( KJob* ) ) );
}

EpodProvider::~EpodProvider()
{
    delete d;
}

QImage EpodProvider::image() const
{
    return d->mImage;
}

QString EpodProvider::identifier() const
{
    return QString( "epod:%1" ).arg( d->mDate.toString( Qt::ISODate ));
}

#include "epodprovider.moc"
