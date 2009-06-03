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
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updatePicture()));
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
    m_picture = QImage();
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
    m_picture = QImage();
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

QImage SlideShow::image()
{
    if (m_picture.isNull()) {
        //kDebug() << m_currentUrl;
        Picture myPicture;
        m_picture = myPicture.setPicture(m_currentUrl);
        // we'll clear the image after a few seconds, but this allows
        // the applet to do it's business without reloading the picture
        // over and over in the meantime
        if (!m_timer->isActive() || m_timer->interval() > 5000) {
            QTimer::singleShot(5000, this, SLOT(clearPicture()));
        }
    }

    return m_picture;
}

KUrl SlideShow::url()
{
    if (!m_picturePaths.isEmpty()) {
        int index = -1;
        if (m_useRandom) {
            m_randomInt++;
            index = m_indexList[m_randomInt % m_picturePaths.count()];
            //kDebug() << "Random was selected and the index was: " << index << " out of " << m_picturePaths.count() << " images" << endl;
        } else {
            index = m_slideNumber++ % m_picturePaths.count();
        }

        return KUrl(m_picturePaths.at(index));
    } else {
        return KUrl("Default");
    }
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

void SlideShow::updatePicture()
{
    m_currentUrl = url();
    m_picture = QImage();
    emit pictureUpdated();
}

void SlideShow::clearPicture()
{
    m_picture = QImage();
}

void SlideShow::dataUpdated(const QString &name, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(name)
    if (data.isEmpty()) {
        m_picture = QImage();
        return;
    }

    m_picture = data[0].value<QImage>();
    emit pictureUpdated();
}

#include "slideshow.moc"

