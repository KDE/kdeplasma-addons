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

    const QString pattern( QLatin1String( "http://epod.usra.edu/.a/*-pi" ) );
    QRegExp exp( pattern );
    exp.setPatternSyntax(QRegExp::Wildcard);

    int pos = exp.indexIn( data ) + pattern.length();
    const QString sub = data.mid( pos-4, pattern.length()+6);
    KUrl url( QString(QLatin1String( "http://epod.usra.edu/.a/%1-pi" )) .arg(sub)  );
    KIO::StoredTransferJob *imageJob = KIO::storedGet( url, KIO::NoReload, KIO::HideProgressInfo );
    QObject::connect(imageJob, SIGNAL(finished(KJob*)), mParent, SLOT(imageRequestFinished(KJob*)) );
}

void EpodProvider::Private::imageRequestFinished( KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if ( job->error() ) {
	emit mParent->error( mParent );
	return;
    }

    // FIXME: this really should be done in a thread as this can block
    mImage = QImage::fromData( job->data() );
    emit mParent->finished( mParent );
}

EpodProvider::EpodProvider( QObject *parent, const QVariantList &args )
    : PotdProvider( parent, args ), d( new Private( this ) )
{
    KUrl url( QLatin1String( "http://epod.usra.edu/blog/" ) );
    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::NoReload, KIO::HideProgressInfo );

    connect( job, SIGNAL(finished(KJob*)), SLOT(pageRequestFinished(KJob*)) );
}

EpodProvider::~EpodProvider()
{
    delete d;
}

QImage EpodProvider::image() const
{
    return d->mImage;
}

#include "epodprovider.moc"
