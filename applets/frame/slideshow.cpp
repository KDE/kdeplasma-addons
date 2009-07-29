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
    m_randomInt = 0;

    m_picture = new Picture(this);
    connect(m_picture, SIGNAL(pictureLoaded(QPixmap)), this, SLOT(pictureLoaded(QPixmap)));

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
    m_image = QPixmap();
    m_picturePaths.clear();
    foreach(const QString &path, slideShowPath) {
        if (recursive) {
            addRecursiveDir(KUrl(path).path());
        } else {
            addDir(KUrl(path).path());
        }
    }
}

void SlideShow::setImage(const QString &imagePath)
{
    m_image = QPixmap();
    m_picturePaths.clear();
    addImage(imagePath);
    m_currentUrl = url();
}

void SlideShow::addImage(const QString &imagePath)
{
    if (!m_picturePaths.contains(imagePath)) {
        m_picturePaths.append(imagePath);
    }
}

void SlideShow::addDir(const QString &path)
{
    QDir dir(path);
    dir.setNameFilters(m_filters);

    if (dir.entryList().isEmpty())  {
        //TODO remove the path from the list
        return;
    }

    foreach(const QString &imageFile, dir.entryList(QDir::Files)) {
        addImage(path + '/' + imageFile);
    }

    KRandomSequence randomSequence;
    m_indexList.clear();

    //get the number of sounds then shuffle it: each number will be taken once then the sequence will come back
    for (int j = 0; j < m_picturePaths.count(); j++) {
        m_indexList.append(j);
    }

    randomSequence.randomize(m_indexList);
}

void SlideShow::addRecursiveDir(const QString &path)
{
    addDir(path);
    QDir dir(path);

    foreach(const QString &subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        addRecursiveDir(path + '/' + subDir);
    }
}

QPixmap SlideShow::image()
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
        if (m_useRandom) {
            m_randomInt += offset;

            if (m_randomInt <= -1) {
                m_randomInt = m_picturePaths.count() - 1;

            } else if (m_randomInt >= m_picturePaths.count()) {
                m_randomInt = 0;
            }

            return KUrl(m_picturePaths.at(m_indexList.at(m_randomInt)));
        }

        m_slideNumber += offset;

        if (m_slideNumber <= -1) {
            m_slideNumber = m_picturePaths.count() - 1;

        } else if (m_slideNumber >= m_picturePaths.count()) {
            m_slideNumber = 0;
        }

        return KUrl(m_picturePaths.at(m_slideNumber));
    }

    return KUrl();
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

QString SlideShow::message()
{
    return m_picture->message();
}

void SlideShow::pictureLoaded(QPixmap image)
{
    m_image = image;
    emit pictureUpdated();
}

void SlideShow::clearPicture()
{
    m_image = QPixmap();
}

void SlideShow::dataUpdated(const QString &name, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(name)
    if (data.isEmpty()) {
        m_image = QPixmap();
        return;
    }

    m_image = data[0].value<QPixmap>();
    emit pictureUpdated();
}

#include "slideshow.moc"

