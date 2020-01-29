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

#include <QDebug>

#include <KPluginFactory>
#include <KIO/Job>


NatGeoProvider::NatGeoProvider(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args)
{
    const QUrl url(QStringLiteral("https://www.nationalgeographic.com/photography/photo-of-the-day/"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &NatGeoProvider::pageRequestFinished);
}

NatGeoProvider::~NatGeoProvider() = default;

QImage NatGeoProvider::image() const
{
    return mImage;
}

void NatGeoProvider::pageRequestFinished(KJob* _job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if (job->error()) {
        emit error(this);
        return;
    }

    const QString data = QString::fromUtf8( job->data() );
    const QStringList lines = data.split(QLatin1Char('\n'));

    QString url;

    re.setPattern(QStringLiteral("^<meta\\s+property=\"og:image\"\\s+content=\"(.*)\".*/>$"));

    for (int i = 0; i < lines.size(); i++) {
        QRegularExpressionMatch match = re.match(lines.at(i));
        if (match.hasMatch()) {
            url = match.captured(1);
        }
    }

    if (url.isEmpty()) {
        emit error(this);
        return;
    }

    KIO::StoredTransferJob *imageJob = KIO::storedGet( QUrl(url), KIO::NoReload, KIO::HideProgressInfo );
    connect(imageJob, &KIO::StoredTransferJob::finished, this, &NatGeoProvider::imageRequestFinished);
}

void NatGeoProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
        emit error(this);
        return;
    }

    mImage = QImage::fromData( job->data() );
    emit finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(NatGeoProvider, "natgeoprovider.json")

#include "natgeoprovider.moc"
