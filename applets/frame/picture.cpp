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

#include <QPainter>
#include <KSvgRenderer>
#include <QTextOption>

#include <KUrl>

#include <klocalizedstring.h>
#include <plasma/theme.h>

#ifdef HAVE_KEXIV2
#include <libkexiv2/kexiv2.h>
#endif

Picture::Picture()
{
}

Picture::~Picture()
{
}

QImage Picture::defaultPicture(const QString &message)
{
    // Create a QImage with same axpect ratio of default svg and current pixelSize
    QString svgFile = Plasma::Theme::defaultTheme()->imagePath("widgets/picture-frame-default");
    KSvgRenderer sr(svgFile);
    QImage imload(sr.defaultSize(), QImage::Format_RGB32);//TODO optimize, too slow

    QPainter p(&imload);
    sr.render(&p, QRect(QPoint(0, 0), imload.size()));

    // Set the font and draw text
    p.setRenderHint(QPainter::Antialiasing);
    QFont textFont;
    textFont.setPixelSize(imload.height() / 12);
    p.setFont(textFont);
    p.setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    QTextOption option;
    option.setAlignment(Qt::AlignCenter);
    option.setWrapMode(QTextOption::WordWrap);
    p.drawText(imload.rect(), message, option);
    p.end();
    return imload;
}

QImage Picture::setPicture(const KUrl &currentUrl)
{
    QImage m_picture;
    if (currentUrl.path() == "Default") {
        m_picture = defaultPicture(i18nc("Info", "Put your photo here or drop a folder to start a slideshow"));
        return m_picture;
    } else {
        QImage tempImage(currentUrl.path());
        if (tempImage.isNull()) {
            m_picture = defaultPicture(i18nc("Error", "Error loading image"));
            return m_picture;
        } else {
#ifdef HAVE_KEXIV2
            KExiv2Iface::KExiv2 exif(currentUrl.path());
            QMatrix m;
            switch (exif.getImageOrientation()) {
                case KExiv2Iface::KExiv2::ORIENTATION_HFLIP: m_picture = tempImage.mirrored(true, false); break;
                case KExiv2Iface::KExiv2::ORIENTATION_ROT_180: m_picture = tempImage.mirrored(true, true); break;
                case KExiv2Iface::KExiv2::ORIENTATION_VFLIP: m_picture = tempImage.mirrored(false, true); break;
                case KExiv2Iface::KExiv2::ORIENTATION_ROT_90: m.rotate(90); m_picture = tempImage.transformed(m); break;
                case KExiv2Iface::KExiv2::ORIENTATION_ROT_90_HFLIP: m.rotate(90); m.scale(-1.0,1.0); 
                                                      m_picture = tempImage.transformed(m); break;
                case KExiv2Iface::KExiv2::ORIENTATION_ROT_90_VFLIP: m.rotate(90); m.scale(1.0,-1.0); 
                                                      m_picture = tempImage.transformed(m); break;
                case KExiv2Iface::KExiv2::ORIENTATION_ROT_270: m.rotate(270);  m_picture = tempImage.transformed(m); break;
                default: m_picture = tempImage;
            }
#else
            m_picture = tempImage;
#endif
            return m_picture;
        }
    }
}
