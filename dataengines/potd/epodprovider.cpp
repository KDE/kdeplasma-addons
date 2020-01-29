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

#include <QRegExp>
#include <QDebug>

#include <KPluginFactory>
#include <KIO/Job>

EpodProvider::EpodProvider( QObject *parent, const QVariantList &args )
    : PotdProvider(parent, args)
{
    const QUrl url(QStringLiteral("https://epod.usra.edu/blog/"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &EpodProvider::pageRequestFinished);
}

EpodProvider::~EpodProvider() = default;

QImage EpodProvider::image() const
{
    return mImage;
}

void EpodProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if ( job->error() ) {
	emit error(this);
	return;
    }

    const QString data = QString::fromUtf8( job->data() );

    const QString pattern = QStringLiteral("://epod.usra.edu/.a/*-pi");
    QRegExp exp( pattern );
    exp.setPatternSyntax(QRegExp::Wildcard);

    int pos = exp.indexIn( data ) + pattern.length();
    const QString sub = data.mid( pos-4, pattern.length()+10);
    const QUrl url(QStringLiteral("https://epod.usra.edu/.a/%1-pi").arg(sub));
    KIO::StoredTransferJob *imageJob = KIO::storedGet( url, KIO::NoReload, KIO::HideProgressInfo );
    connect(imageJob, &KIO::StoredTransferJob::finished, this, &EpodProvider::imageRequestFinished);
}

void EpodProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if ( job->error() ) {
	emit error(this);
	return;
    }

    // FIXME: this really should be done in a thread as this can block
    mImage = QImage::fromData( job->data() );
    emit finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(EpodProvider, "epodprovider.json")

#include "epodprovider.moc"
