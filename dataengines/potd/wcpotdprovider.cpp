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
    QImage mImage;
};

void WcpotdProvider::Private::pageRequestFinished( KJob *_job )
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit mParent->error( mParent );
	return;
    }

    KUrl picUrl( QLatin1String( "http://tools.wikimedia.de/~daniel/potd/potd.php/commons/800x600" ) );  KIO::StoredTransferJob *imageJob = KIO::storedGet( picUrl, KIO::NoReload, KIO::HideProgressInfo );
    mParent->connect( imageJob, SIGNAL(finished(KJob*)), SLOT(imageRequestFinished(KJob*)) );
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
    KUrl url( QLatin1String( "http://tools.wikimedia.de/~daniel/potd/potd.php/commons/800x600" ));

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::NoReload, KIO::HideProgressInfo );
    connect( job, SIGNAL(finished(KJob*)), SLOT(pageRequestFinished(KJob*)) );
}

WcpotdProvider::~WcpotdProvider()
{
    delete d;
}

QImage WcpotdProvider::image() const
{
    return d->mImage;
}

#include "wcpotdprovider.moc"
