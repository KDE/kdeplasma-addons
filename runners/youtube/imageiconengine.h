/*
 *   Copyright (C) 2009 Christoph Feck <christoph@maxiom.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef IMAGEICONENGINE_H
#define IMAGEICONENGINE_H

#include <QtGui/QIconEngineV2>
#include <QtGui/QIcon>

class ImageIconEngine : public QIconEngineV2
{
    public:
        ImageIconEngine(const QImage &image);
        ~ImageIconEngine();

        QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state);
        void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state);
        QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state);

    private:
        QImage m_image;
};

#endif
