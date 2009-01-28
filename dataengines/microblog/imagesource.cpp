/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
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

#include "imagesource.h"

#include <QBuffer>

#include <KIO/Job>

ImageSource::ImageSource(QObject* parent)
    : Plasma::DataContainer(parent)
{
    setObjectName("UserImages");
}

ImageSource::~ImageSource()
{
}

void ImageSource::loadImage(const QString &who, const KUrl &url)
{
    KIO::Job *job = KIO::get(url, KIO::Reload, KIO::HideProgressInfo);
    m_jobs[job] = who;
    connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(recv(KIO::Job*, const QByteArray&)));
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void ImageSource::recv(KIO::Job* job, const QByteArray& data)
{
    m_jobData[job] += data;
    //kDebug() << m_data;
}

void ImageSource::result(KJob *job)
{
    if (!m_jobs.contains(job)) {
        return;
    }

    if (job->error()) {
        // TODO: error handling
    } else {
        //kDebug() << "done!" << m_jobData;
        QImage img;
        img.loadFromData(m_jobData.value(job));
        setData(m_jobs.value(job), img);
    }

    m_jobs.remove(job);
    m_jobData.remove(job);
    checkForUpdate();
}

#include <imagesource.moc>

