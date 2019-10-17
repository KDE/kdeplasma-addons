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

#include "noaaprovider.h"

#include <QRegularExpression>
#include <QDebug>

#include <KPluginFactory>
#include <KIO/Job>

NOAAProvider::NOAAProvider(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args)
{
    const QUrl url(QStringLiteral("https://www.nesdis.noaa.gov/content/imagery-and-data"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &NOAAProvider::pageRequestFinished);
}

NOAAProvider::~NOAAProvider() = default;

QImage NOAAProvider::image() const
{
    return mImage;
}

void NOAAProvider::pageRequestFinished(KJob* _job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if (job->error()) {
        emit error(this);
        return;
    }

    const QString data = QString::fromUtf8( job->data() );

    // Using regular expression could be fragile in such case, but the HTML
    // NOAA page itself is not a valid XML file and unfortunately it could
    // not be parsed successfully till the content we want. And we do not want
    // to use heavy weight QtWebkit. So we use QRegularExpression to capture
    // the wanted url here.
    QUrl url;
    QRegularExpression re(QStringLiteral("\"(/sites/default/files/.*\\.jpg)\""));
    auto result = re.match(data);
    if (result.hasMatch()) {
        url = QUrl(QStringLiteral("https://www.nesdis.noaa.gov") + result.captured(1));
    }
    if (!url.isValid()) {
        emit error(this);
        return;
    }

    KIO::StoredTransferJob *imageJob = KIO::storedGet( url, KIO::NoReload, KIO::HideProgressInfo );
    connect(imageJob, &KIO::StoredTransferJob::finished, this, &NOAAProvider::imageRequestFinished);
}

void NOAAProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit error(this);
	return;
    }

    mImage = QImage::fromData( job->data() );
    emit finished(this);
}

K_PLUGIN_CLASS_WITH_JSON(NOAAProvider, "noaaprovider.json")

#include "noaaprovider.moc"
