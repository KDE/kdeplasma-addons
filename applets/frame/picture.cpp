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
#include <QSvgRenderer>

#include <KUrl>
#include <KStandardDirs>

Picture::Picture()
{
}

Picture::~Picture()
{
}

QImage Picture::defaultPicture(int pixelSize, const QString &message)
{
    // Create a QImage with same axpect ratio of default svg and current pixelSize
    QString svgFile = Plasma::Theme::self()->image("widgets/picture-frame-default");
    QSvgRenderer sr(svgFile);
    QImage imload(sr.defaultSize(),QImage::Format_RGB32);//TODO optimize, too slow
   
    QPainter p(&imload);
    sr.render(&p, QRect(QPoint(0, 0), imload.size()));

    // Set the font and draw text 
    p.setRenderHint(QPainter::Antialiasing);
    QFont textFont;
    textFont.setPixelSize(imload.height() / 12);
    p.setFont(textFont);
    p.drawText(imload.rect(), Qt::AlignCenter, message);
    p.end();
    return imload;
}

QImage Picture::setPicture(int pixelSize, const KUrl &currentUrl)
{
    QImage m_picture;
    if (currentUrl.path() == "Default") {
	m_picture = defaultPicture(pixelSize, "Put your photo here\nor drop a folder\nfor starting a slideshow");
	return m_picture;
    } else {
        QImage tempImage(currentUrl.path());
        if (tempImage.isNull()){
            m_picture = defaultPicture(pixelSize, "Error loading image");
	    return m_picture;
        } else { // Load success! Scale the image if it is too big	    
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

