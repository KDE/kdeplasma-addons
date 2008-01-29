/***************************************************************************
 *   Copyright  2008 by Anne-Marie Mahfouf <annma@kde.org>                 *
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

#include <QDir>
#include <QPainter>
#include <QPixmap>

#include <KUrl>
#include <KStandardDirs>


Picture::Picture()
{
}

Picture::~Picture()
{
}

QImage Picture::defaultPicture(const QString &message)
{
    // Create a QImage with same axpect ratio of default svg and current pixelSize
    QString defaultFile = KGlobal::dirs()->findResource("data", "plasma-frame/picture-frame-default.jpg");
    QImage imload; 
    imload.load( defaultFile);
    // Write message
    QPainter p(&imload);
    p.setRenderHint(QPainter::Antialiasing);
    QFont textFont;
    textFont.setPixelSize(imload.height() / 10);
    p.setFont(textFont);
    p.drawText(imload.rect(), Qt::AlignCenter, message);
    p.end();
    return imload;
}

QImage Picture::setPicture(KUrl currentUrl)
{
    if (currentUrl.url().isEmpty()) {
	return defaultPicture("Put your photo here\nor drop a folder\nfor starting a slideshow");
    } else {
        QImage tempImage(currentUrl.path());
        if (tempImage.isNull()){
            return defaultPicture("Error loading image");
        } else { // Load success! Scale the image if it is too big
	    QImage m_picture;
            if (tempImage.width() > m_maxDimension || tempImage.height() > m_maxDimension) {
                m_picture = tempImage.scaled(m_maxDimension,m_maxDimension,
                                             Qt::KeepAspectRatio,Qt::SmoothTransformation);
            } else {
                m_picture = tempImage;
            }
	    return m_picture;
        }
    }
}

QStringList Picture::findSlideShowPics(const QStringList &slideShowPaths)
{
    QStringList picList;

    foreach (const QString &path, slideShowPaths) {
        QDir dir(path);
        QStringList filters;
        filters << "*.jpeg" << "*.jpg" << "*.png" << "*.svg" << "*.svgz"; // use mime types?
        dir.setNameFilters(filters);
        foreach (const QString &imageFile, dir.entryList(QDir::Files)) {
            picList.append(path + "/" + imageFile);
        }
    }
    return picList;
}
