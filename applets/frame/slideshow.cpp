/***************************************************************************
 *   Copyright  2008 by Thomas Coopman <thomas.coopman@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "slideshow.h"

#include <QDir>
#include <QDirIterator>
#include <QTimer>

#include <KDebug>
#include <KRandomSequence>

#include "picture.h"

SlideShow::SlideShow(QObject *parent)
    : QObject(parent)
{
    m_filters << "*.jpeg" << "*.jpg" << "*.png" << "*.svg" << "*.svgz"; // use mime types?
    m_slideNumber = 0;
    m_useRandom = false;

    m_picture = new Picture(this);
    connect(m_picture, SIGNAL(pictureLoaded(QImage)), this, SLOT(pictureLoaded(QImage)));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(nextPicture()));
}

SlideShow::~SlideShow()
{
}

void SlideShow::setRandom(bool useRandom)
{
    m_useRandom = useRandom;
}

void SlideShow::setDirs(const QStringList &slideShowPath, bool recursive)
{
    QDateTime setDirStart = QDateTime::currentDateTime();

    m_image = QImage();
    m_picturePaths.clear();
    foreach(const QString &path, slideShowPath) {
        addDir(KUrl(path).path(), recursive);
    }

    KRandomSequence randomSequence;
    m_indexList.clear();

    for (int j = 0; j < m_picturePaths.count(); j++) {
        m_indexList.append(j);
    }
    randomSequence.randomize(m_indexList);

    // select 1st picture
    firstPicture();
    
    // TODO do something meaningful if m_picturePaths.empty()
    
    kDebug() << "Loaded " << m_picturePaths.size() << " pictures in " << setDirStart.secsTo(QDateTime::currentDateTime()) << " seconds";
}

void SlideShow::setImage(const QString &imagePath)
{
    m_image = QImage();
    m_picturePaths.clear();
    addImage(imagePath);
    m_currentUrl = url();
}

void SlideShow::addImage(const QString &imagePath)
{
    m_picturePaths.append(imagePath);
}

void SlideShow::addDir(const QString &path, bool recursive)
{
    QDirIterator dirIterator(path, m_filters, QDir::Files, (recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags));
    QStringList dirPicturePaths;

    while (dirIterator.hasNext()) {
        dirIterator.next();
        dirPicturePaths.append(dirIterator.filePath());
    }
    
    // the pictures have to be sorted before adding them to the list,
    // because the QDirIterator sorts them in a different way than QDir::entryList
    dirPicturePaths.sort();
    m_picturePaths.append(dirPicturePaths);
}

QImage SlideShow::image() const
{
    if (m_image.isNull() || m_currentUrl != m_picture->url()) {
        kDebug() << "reloading from Picture" << m_currentUrl;
        //m_currentUrl = m_picture->url();
        m_picture->setPicture(m_currentUrl);
    }
    kDebug();
    return m_image;
}

KUrl SlideShow::url(int offset)
{
    if (!m_picturePaths.isEmpty()) {

        m_slideNumber += offset;

        if (m_slideNumber <= -1) {
            m_slideNumber = m_picturePaths.count() - 1;

        } else if (m_slideNumber >= m_picturePaths.count()) {
            m_slideNumber = 0;
        }

        if (m_useRandom) {
            return KUrl(m_picturePaths.at(m_indexList.at(m_slideNumber)));
        } else {
            return KUrl(m_picturePaths.at(m_slideNumber));
        }
    }

    return KUrl();
}

void SlideShow::firstPicture()
{
    m_slideNumber = 0;
    m_currentUrl = url(0);
    m_image = image();
    emit pictureUpdated();
}

void SlideShow::nextPicture()
{
    m_currentUrl = url(1);
    m_image = image();
    emit pictureUpdated();
}

void SlideShow::previousPicture()
{
    m_currentUrl = url(-1);
    m_image = image();
    emit pictureUpdated();
}

KUrl SlideShow::currentUrl() const
{
    return m_currentUrl;
}

void SlideShow::setUpdateInterval(int msec)
{
    m_timer->stop();
    if (msec > 1) {
        if (m_currentUrl.isEmpty()) {
            m_currentUrl = url();
        }
        m_timer->start(msec);
    }
}

QString SlideShow::message() const
{
    return m_picture->message();
}

void SlideShow::pictureLoaded(QImage image)
{
    m_image = image;
    emit pictureUpdated();
}

void SlideShow::clearPicture()
{
    m_image = QImage();
}

void SlideShow::dataUpdated(const QString &name, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(name)
    if (data.isEmpty()) {
        m_image = QImage();
        return;
    }

    m_image = data[0].value<QImage>();
    //Compatibility with old dataengines
    if (m_image.isNull()){
        QPixmap tmpPixmap = data[0].value<QPixmap>();
        if (!tmpPixmap.isNull()){
            m_image = tmpPixmap.toImage();
        }
    }
    emit pictureUpdated();
}

#include "slideshow.moc"

