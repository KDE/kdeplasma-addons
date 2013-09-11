/*
 *   Copyright 2007 Tobias Koenig <tokoe@kde.org>
 *   Copyright 2008 Anne-Marie Mahfouf <annma@kde.org>
 *   Copyright 2013 Aaron Seigo <aseigo@kde.org>
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

#include "natgeoprovider.h"

#include <QtCore/QRegExp>
#include <QtGui/QImage>

#include <KDebug>
#include <KIO/Job>

#include <KWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QWebElementCollection>

POTDPROVIDER_EXPORT_PLUGIN( NatGeoProvider, "NatGeoProvider", "" )

class NatGeoProvider::Private
{
  public:
    Private( NatGeoProvider *parent )
      : mParent( parent ),
        mPage( new KWebPage( parent, KWebPage::KIOIntegration ) )
    {
    }

    void pageRequestFinished( bool ok );
    void imageRequestFinished( KJob* );
    void parsePage();

    NatGeoProvider *mParent;
    QImage mImage;
    KWebPage *mPage;
};

void NatGeoProvider::Private::pageRequestFinished( bool ok )
{
    if ( !ok ) {
        emit mParent->error( mParent );
        return;
    }

    QWebElementCollection links = mPage->mainFrame()->documentElement().findAll( QLatin1String( "div.download_link a" ) );
    if ( links.count() < 1 ) {
        emit mParent->error( mParent );
        return;
    }

    const QString url = links.at(0).attribute( QLatin1String( "href" ) );
    if (url.isEmpty()) {
        emit mParent->error( mParent );
        return;
    }


    KIO::StoredTransferJob *imageJob = KIO::storedGet( url, KIO::NoReload, KIO::HideProgressInfo );
    mParent->connect( imageJob, SIGNAL(finished(KJob*)), SLOT(imageRequestFinished(KJob*)) );
}

void NatGeoProvider::Private::imageRequestFinished( KJob *_job )
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
        emit mParent->error( mParent );
        return;
    }

    mImage = QImage::fromData( job->data() );
    emit mParent->finished( mParent );
}

NatGeoProvider::NatGeoProvider( QObject *parent, const QVariantList &args )
    : PotdProvider( parent, args ), d( new Private( this ) )
{
    const QUrl url( QLatin1String( "http://photography.nationalgeographic.com/photography/photo-of-the-day" ) );
    connect( d->mPage, SIGNAL(loadFinished(bool)), this, SLOT(pageRequestFinished(bool)) );
    d->mPage->mainFrame()->setUrl( url );
}

NatGeoProvider::~NatGeoProvider()
{
    delete d;
}

QImage NatGeoProvider::image() const
{
    return d->mImage;
}

#include "natgeoprovider.moc"
