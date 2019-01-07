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

#include <QRegExp>
#include <QDebug>

#include <KPluginFactory>
#include <KIO/Job>

ApodProvider::ApodProvider(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args)
{
    const  QUrl url(QStringLiteral("http://antwrp.gsfc.nasa.gov/apod/"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &ApodProvider::pageRequestFinished);
}

ApodProvider::~ApodProvider() = default;

QImage ApodProvider::image() const
{
    return mImage;
}

void ApodProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
        emit error(this);
        return;
    }

    const QString data = QString::fromUtf8( job->data() );

    const QString pattern = QStringLiteral("<a href=\"(image/.*)\"" );
    QRegExp exp( pattern );
    exp.setMinimal( true );
    if ( exp.indexIn( data ) != -1 ) {
        const QString sub = exp.cap(1);
        const QUrl url(QLatin1String("http://antwrp.gsfc.nasa.gov/apod/") + sub);
        KIO::StoredTransferJob *imageJob = KIO::storedGet( url, KIO::NoReload, KIO::HideProgressInfo );
        connect(imageJob, &KIO::StoredTransferJob::finished, this, &ApodProvider::imageRequestFinished);
    } else {
        emit error(this);
    }
}

void ApodProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit error(this);
	return;
    }

    mImage = QImage::fromData( job->data() );
    emit finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(ApodProvider, "apodprovider.json")

#include "apodprovider.moc"
