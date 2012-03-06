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

#include "imageiconengine.h"

#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

ImageIconEngine::ImageIconEngine(const QImage &image)
    : m_image(image)
{
}

ImageIconEngine::~ImageIconEngine()
{
}

QSize ImageIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode);
    Q_UNUSED(state);

    return size;
}

void ImageIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    painter->fillRect(rect, Qt::transparent);
    QPixmap p = pixmap(rect.size(), mode, state);
    QRect r = p.rect();
    r.moveCenter(rect.center());
    painter->drawPixmap(r, p);
}

QPixmap ImageIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(state);

    QImage image = m_image;
    if (size.isValid() && size != image.size()) {
        image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QStyleOption opt;
    return QApplication::style()->generatedIconPixmap(mode, QPixmap::fromImage(image), &opt);
}

