/***************************************************************************
 *   Copyright  2010 by Davide Bettio <davide.bettio@kdemail.net>          *
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

#include "imageloader.h"

#include <QImage>
#include <kdebug.h>

#ifdef HAVE_KEXIV2
#include <libkexiv2/kexiv2.h>
#endif

ImageLoader::ImageLoader(const QString &path)
{
    m_path = path;
}


QImage ImageLoader::correctRotation(const QImage& tempImage, const QString &path)
{
    QImage image = QImage();
    if (!tempImage.isNull()){
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

void ImageLoader::run()
{
    QImage img = correctRotation(QImage(m_path), m_path);

    emit loaded(img);
}

#include "imageloader.moc"
