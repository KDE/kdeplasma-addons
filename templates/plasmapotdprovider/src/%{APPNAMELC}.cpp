/*
 *   Copyright (C) %{CURRENT_YEAR} by %{AUTHOR} <%{EMAIL}>                      *
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

#include "%{APPNAMELC}.h"

// KF
#include <KPluginFactory>
#include <KIO/Job>


%{APPNAME}::%{APPNAME}(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args)
{
    // TODO: replace with url to data about what the current picture of the day is
    const QUrl potdFeed(QStringLiteral("https://kde.org"));

    KIO::StoredTransferJob* job = KIO::storedGet(potdFeed, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished,
            this, &%{APPNAME}::handleFinishedFeedRequest);
}

%{APPNAME}::~%{APPNAME}()
{
}

QImage %{APPNAME}::image() const
{
    return mImage;
}

void %{APPNAME}::handleFinishedFeedRequest(KJob *job)
{
    KIO::StoredTransferJob *requestJob = static_cast<KIO::StoredTransferJob*>(job);
    if (requestJob->error()) {
        emit error(this);
        return;
    }

    // TODO: read url to image from requestJob->data()
    const QUrl picureUrl(QStringLiteral("https://techbase.kde.org/favicon.png"));

    KIO::StoredTransferJob *imageJob = KIO::storedGet(picureUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(imageJob, &KIO::StoredTransferJob::finished,
            this, &%{APPNAME}::handleFinishedImageRequest);
}

void %{APPNAME}::handleFinishedImageRequest(KJob *job)
{
    KIO::StoredTransferJob *requestJob = static_cast<KIO::StoredTransferJob*>(job);
    if (requestJob->error()) {
        emit error(this);
        return;
    }

    mImage = QImage::fromData(requestJob->data());

    if (mImage.isNull()) {
        emit error(this);
        return;
    }

    emit finished(this);
}


K_PLUGIN_CLASS_WITH_JSON(%{APPNAME}, "%{APPNAMELC}.json")

#include "%{APPNAMELC}.moc"
