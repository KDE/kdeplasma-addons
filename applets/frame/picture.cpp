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
#include <QPainter>
#include <KSvgRenderer>
#include <QTextOption>

#include <KGlobalSettings>
#include <KStandardDirs>
#include <KUrl>
#include <KIO/NetAccess>
#include <KIO/Job>

#include <klocalizedstring.h>
#include <Plasma/Theme>
#include <Plasma/Svg>

#ifdef HAVE_KEXIV2
#include <libkexiv2/kexiv2.h>
#endif

Picture::Picture(QObject *parent)
: QObject(parent)
{
    m_message = QString();
    m_defaultImage = KGlobal::dirs()->findResource("data", "plasma-applet-frame/picture-frame-default.jpg");

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

QPixmap Picture::defaultPicture(const QString &message)
{
    // Create a QImage with same axpect ratio of default svg and current pixelSize

    kDebug() << "Default Image:" << m_defaultImage;
    QPixmap image = QPixmap(m_defaultImage);
    m_message = message;
    return image;
}

void Picture::setPicture(const KUrl &currentUrl)
{
    m_currentUrl = currentUrl;
    QPixmap image;
    kDebug() << currentUrl;
    if (!m_currentUrl.isEmpty() && !m_currentUrl.isLocalFile()) {
        kDebug() << "Not a local file, downloading" << currentUrl;
        m_job = KIO::storedGet( currentUrl, KIO::NoReload, KIO::DefaultFlags );
        connect(m_job, SIGNAL(finished(KJob*)), this, SLOT(slotFinished(KJob*)));
        image = QPixmap(m_defaultImage);
        m_message = i18n("Loading image ...");
    } else {
        if (currentUrl.isEmpty() || currentUrl.path() == "Default") {
            image = QPixmap(m_defaultImage);
            m_message = i18nc("Info", "Put your photo here or drop a folder to start a slideshow");
            kDebug() << "default image ...";
        } else {
            image = correctRotation(QPixmap(m_currentUrl.path()), m_currentUrl.path());
            setPath(m_currentUrl.path());
            m_message = QString();
        }
    }
    emit pictureLoaded(image);
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
    emit pictureLoaded(correctRotation(QPixmap(m_path), m_path));
}

QPixmap Picture::correctRotation(QPixmap tempImage, const QString &path)
{
    kDebug() << path;
    QPixmap image = QPixmap();
    if (tempImage.isNull()) {
        image = defaultPicture(i18nc("Error", "Error loading image"));
    } else {
#ifdef HAVE_KEXIV2
        KExiv2Iface::KExiv2 exif(path);
        QMatrix m;
        switch (exif.getImageOrientation()) {
            case KExiv2Iface::KExiv2::ORIENTATION_HFLIP: m.scale(-1.0,1.0); image = tempImage.transformed(m); break;
            case KExiv2Iface::KExiv2::ORIENTATION_ROT_180: m.rotate(180);  image = tempImage.transformed(m); break;
            case KExiv2Iface::KExiv2::ORIENTATION_VFLIP: m.scale(1.0,-1.0); image = tempImage.transformed(m); break;
            case KExiv2Iface::KExiv2::ORIENTATION_ROT_90: m.rotate(90); image = tempImage.transformed(m); break;
            case KExiv2Iface::KExiv2::ORIENTATION_ROT_90_HFLIP: m.rotate(90); m.scale(-1.0,1.0);
            image = tempImage.transformed(m); break;
            case KExiv2Iface::KExiv2::ORIENTATION_ROT_90_VFLIP: m.rotate(90); m.scale(1.0,-1.0);
            image = tempImage.transformed(m); break;
            case KExiv2Iface::KExiv2::ORIENTATION_ROT_270: m.rotate(270);  image = tempImage.transformed(m); break;
            default: image = tempImage;
        }
#else
        image = tempImage;
#endif
    }
    return image;
}

void Picture::slotFinished( KJob *job )
{
    QString filename = m_currentUrl.fileName();
    QString path = KGlobalSettings::downloadPath() + m_currentUrl.fileName();
    QPixmap image;
    if (job->error()) {
        kDebug() << "Error loading image:" << job->errorString();
        image = defaultPicture(i18n("Error loading image: %1", job->errorString()));
    } else {
        image.loadFromData(m_job->data());
        kDebug() << "Successfully downloaded, saving image to" << path;
        m_message = QString();
        int i = 1;
        while (QFile(path).exists()) {
            // the file exists, prepend $number- to the filename and count up
            QString tmppath = path;
            tmppath.replace(filename, QString("%1-%2").arg(QString("%1").arg(i), filename));
            if (!QFile(tmppath).exists()) {
                path = tmppath;
            } else {
                i++;
            }
        }
        image.save(path);
        kDebug() << "Saved to" << path;
        setPath(path);
    }
    emit pictureLoaded(correctRotation(image, path));
}

#include "picture.moc"