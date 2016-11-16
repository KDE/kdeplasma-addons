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
#include <QXmlStreamReader>

#include <QDebug>
#include <KIO/Job>

class NatGeoProvider::Private
{
  public:
    Private( NatGeoProvider *parent )
      : mParent( parent )
    {
    }

    void pageRequestFinished( KJob* );
    void imageRequestFinished( KJob* );

    NatGeoProvider *mParent;
    QImage mImage;

    QXmlStreamReader mXmlReader;
};

void NatGeoProvider::Private::pageRequestFinished( KJob* _job )
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if (job->error()) {
        emit mParent->error( mParent );
        return;
    }

    const QString data = QString::fromUtf8( job->data() );

    mXmlReader.clear();
    mXmlReader.addData(data);

    QString url;
    while (!mXmlReader.atEnd()) {
        mXmlReader.readNext();

        if (mXmlReader.isStartElement() && mXmlReader.name() == QLatin1String( "meta" )) {
            const auto attrs = mXmlReader.attributes();
            if (attrs.value(QLatin1String("property")).toString() == QLatin1String("og:image")) {
                url = attrs.value(QLatin1String("content")).toString();
                break;
            }
        }
    }

    if (url.isEmpty()) {
        emit mParent->error( mParent );
        return;
    }

    KIO::StoredTransferJob *imageJob = KIO::storedGet( QUrl(url), KIO::NoReload, KIO::HideProgressInfo );
    mParent->connect( imageJob, &KIO::StoredTransferJob::finished, mParent, [this] (KJob *job) {
        imageRequestFinished(job);
    });
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
    const QUrl url( QLatin1String( "https://www.nationalgeographic.com/photography/photo-of-the-day" ) );
    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::NoReload, KIO::HideProgressInfo );
    connect( job, &KIO::StoredTransferJob::finished, this, [this] (KJob *job) {
        d->pageRequestFinished(job);
    });
}

NatGeoProvider::~NatGeoProvider()
{
    delete d;
}

QImage NatGeoProvider::image() const
{
    return d->mImage;
}

K_PLUGIN_FACTORY_WITH_JSON(NatGeoProviderFactory, "natgeoprovider.json", registerPlugin<NatGeoProvider>();)

#include "natgeoprovider.moc"
