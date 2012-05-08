/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2012 Sebastian Kügler <sebas@kde.org>
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

#include <KIcon>
#include <QPainter>
#include <QPainterPath>

#include <KIO/Job>
#include <KImageCache>

ImageSource::ImageSource(QObject* parent)
    : Plasma::DataContainer(parent),
      m_runningJobs(0),
      m_imageCache(0)
{
    setObjectName(QLatin1String("UserImages"));
}

ImageSource::~ImageSource()
{
}

void ImageSource::loadImage(const QString &who, const KUrl &url)
{
    if (who.isEmpty()) {
        return;
    }
    if (!m_imageCache) {
        m_imageCache = new KImageCache("plasma_engine_preview", 10485760); // Re-use previewengine's cache
    }

    // Make sure we only start one job per user
    if (m_loadedPersons.contains(who)) {
        return;
    }

    const QString cacheKey = who + "@" + url.pathOrUrl();

    // Check if the image is in the cache, if so return it
    QImage preview = QImage(QSize(48, 48), QImage::Format_ARGB32_Premultiplied);
    preview.fill(Qt::transparent);
    if (m_imageCache->findImage(cacheKey, &preview)) {
        // cache hit
        //kDebug() << "cache hit: " << cacheKey;
        setData(who, polishImage(preview));
        emit dataChanged();
        checkForUpdate();
        return;
    }
    if (!url.isValid()) {
        return;
    }
    m_loadedPersons << who;
    //FIXME: since kio_http bombs the system with too many request put a temporary
    // arbitrary limit here, revert as soon as BUG 192625 is fixed
    // Note: seems fixed.
    if (m_runningJobs < 500) {
        m_runningJobs++;
        KIO::Job *job = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
        job->setAutoDelete(true);
        m_jobs[job] = who;
        connect(job, SIGNAL(data(KIO::Job*,QByteArray)),
                this, SLOT(recv(KIO::Job*,QByteArray)));
        connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
        job->start();
    } else {
        m_queuedJobs.append(QPair<QString, KUrl>(who, url));
    }
}

void ImageSource::recv(KIO::Job* job, const QByteArray& data)
{
    m_jobData[job] += data;
}

void ImageSource::result(KJob *job)
{
    if (!m_jobs.contains(job)) {
        return;
    }

    m_runningJobs--;

    if (m_queuedJobs.count() > 0) {
        QPair<QString, KUrl> jobDesc = m_queuedJobs.takeLast();
        loadImage(jobDesc.first, jobDesc.second);
    }

    if (job->error()) {
        // TODO: error handling
        KIO::TransferJob* kiojob = dynamic_cast<KIO::TransferJob*>(job);
        kError() << "Image job returned error: " << kiojob->errorString();
    } else {
        QImage img;
        img.loadFromData(m_jobData.value(job));
        const QString who = m_jobs.value(job);

        setData(who, polishImage(img));
        emit dataChanged();
        KIO::TransferJob* kiojob = dynamic_cast<KIO::TransferJob*>(job);
        const QString cacheKey = who + "@" + kiojob->url().pathOrUrl();

        m_imageCache->insertImage(cacheKey, img);
    }

    m_jobs.remove(job);
    m_jobData.remove(job);
    checkForUpdate();
}

QImage ImageSource::polishImage(const QImage &img)
{
    QImage roundedImage = QImage(QSize(48*4, 48*4), QImage::Format_ARGB32_Premultiplied);
    roundedImage.fill(Qt::transparent);
    QPainter p;
    p.begin(&roundedImage);
    QPainterPath clippingPath;
    QRectF imgRect = QRectF(QPoint(0,0), roundedImage.size());
    clippingPath.addRoundedRect(imgRect, 24, 24);
    p.setClipPath(clippingPath);
    p.setClipping(true);
    p.drawImage(QRectF(QPointF(0, 0), roundedImage.size()), img);
    return roundedImage;
}

#include <imagesource.moc>
