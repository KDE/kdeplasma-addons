/***************************************************************************
 *   Copyright  2008 by Anne-Marie Mahfouf <annma@kde.org>                 *
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

#include "picture.h"

#include <QFile>
#include <QThreadPool>

#include <KDebug>
#include <KGlobalSettings>
#include <KUrl>
#include <KIO/NetAccess>
#include <KIO/Job>
#include <KStandardDirs>

#include <klocalizedstring.h>
#include <Plasma/Theme>
#include <Plasma/Svg>

#include "imageloader.h"

Picture::Picture(QObject *parent)
        : QObject(parent)
{
    m_defaultImage = KGlobal::dirs()->findResource("data", "plasma-applet-frame/picture-frame-default.jpg");
    m_checkDir = false;

    // listen for changes to the file we're displaying
    m_fileWatch = new KDirWatch(this);
    connect(m_fileWatch,SIGNAL(dirty(QString)),this,SLOT(reload()));
    connect(m_fileWatch,SIGNAL(created(QString)),this,SLOT(reload()));
    connect(m_fileWatch,SIGNAL(deleted(QString)),this,SLOT(reload()));
}

Picture::~Picture()
{
}

QString Picture::message()
{
    return m_message;
}

void Picture::setMessage(const QString &message)
{
    m_message = message;
}

void Picture::setAllowNullImages(bool allowNull)
{
    m_allowNullImages = allowNull;
}

bool Picture::allowNullImages() const
{
    return m_allowNullImages;
}

QImage Picture::defaultPicture(const QString &message)
{
    // Create a QImage with same axpect ratio of default svg and current pixelSize
    kDebug() << "Default Image:" << m_defaultImage;
    QImage image = QImage(m_defaultImage);
    m_message = message;
    return image;
}

void Picture::setPicture(const KUrl &currentUrl)
{
    m_currentUrl = currentUrl;
    kDebug() << currentUrl;

    if (!m_currentUrl.isEmpty() && !m_currentUrl.isLocalFile()) {
        kDebug() << "Not a local file, downloading" << currentUrl;
        KIO::StoredTransferJob * job = KIO::storedGet( currentUrl, KIO::NoReload, KIO::HideProgressInfo);
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotFinished(KJob*)));
        emit pictureLoaded(defaultPicture(i18n("Loading image...")));
    } else {
        ImageLoader *loader = 0;
        if (m_checkDir) {
            m_message = i18nc("Info", "Dropped folder is empty. Please drop a folder with image(s)");
            m_checkDir = false;
        } else if (currentUrl.isEmpty()) {
            m_message = i18nc("Info", "Put your photo here or drop a folder to start a slideshow");
            kDebug() << "default image ...";
        } else {
            loader = new ImageLoader(m_currentUrl.path());
            setPath(m_currentUrl.path());
            m_message.clear();
        }

        if (!loader) {
            loader = new ImageLoader(m_defaultImage);
        }

        connect(loader, SIGNAL(loaded(QImage)), this, SLOT(checkImageLoaded(QImage)));
        QThreadPool::globalInstance()->start(loader);
    }
}

KUrl Picture::url()
{
    return m_currentUrl;
}

void Picture::setPath(const QString &path)
{
    // Now switch the file watch to the new path
    if (m_path != path) {
        m_fileWatch->removeFile(m_path);
        kDebug() << "-" << m_path;
        m_path = path;
        m_fileWatch->addFile(m_path);
        kDebug() << "+" << m_path;
    }
}

void Picture::reload()
{
    kDebug() << "Picture reload";
    setMessage(QString());
    ImageLoader *loader = new ImageLoader(m_path);
    connect(loader, SIGNAL(loaded(QImage)), this, SLOT(checkImageLoaded(QImage)));
    QThreadPool::globalInstance()->start(loader);
}

void Picture::customizeEmptyMessage() 
{
    m_checkDir = true;
}

void Picture::slotFinished( KJob *job )
{
    QString filename = m_currentUrl.fileName();
    QString path = KStandardDirs::locateLocal("cache", "plasma-frame/" +  m_currentUrl.fileName());
    QImage image;

    if (job->error()) {
        kDebug() << "Error loading image:" << job->errorString();
        image = defaultPicture(i18n("Error loading image: %1", job->errorString()));
    } else if (KIO::StoredTransferJob * transferJob = qobject_cast<KIO::StoredTransferJob *>(job)) {
        image.loadFromData(transferJob->data());
        kDebug() << "Successfully downloaded, saving image to" << path;
        m_message.clear();
        image.save(path);
        kDebug() << "Saved to" << path;
        setPath(path);
    }

    emit checkImageLoaded(ImageLoader::correctRotation(image, path));
}

void Picture::checkImageLoaded(const QImage &newImage)
{
    if (!m_allowNullImages && newImage.isNull()) {
        emit pictureLoaded(defaultPicture(i18n("Error loading image. Image was probably deleted.")));
    } else {
        emit pictureLoaded(newImage);
    }
}

#include "picture.moc"
