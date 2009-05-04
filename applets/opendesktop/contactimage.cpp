/*
    Copyright 2008-2009 by Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

//Qt
#include <QGraphicsWidget>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

// KDE
#include <KIcon>

// Plasma
#include <Plasma/Theme>

// own
#include "contactimage.h"

using namespace Plasma;

ContactImage::ContactImage(QGraphicsWidget *parent)
    : QGraphicsWidget(parent)
{
    border = 1; // should be a power of two, otherwise we get blurry lines
    fg = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    bg = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    m_pixmap = KIcon("system-users").pixmap(64, 64);

}

ContactImage::~ContactImage()
{
}

void ContactImage::setImage(const QImage &image)
{
    if (!image.size().isEmpty()) {
        m_pixmap = QPixmap::fromImage(image);
        pixmapUpdated();
        kDebug() << "----" << m_pixmap.size() << m_scaledPixmap.size();
    }
}

void ContactImage::setPixmap(const QPixmap &pixmap)
{
    if (!pixmap.size().isEmpty()) {
        m_pixmap = pixmap;
        pixmapUpdated();
        kDebug() << "----" << m_pixmap.size() << m_scaledPixmap.size();
    }
}

void ContactImage::pixmapUpdated()
{
    QSize newsize = QSize(contentsRect().width() - (border * 2), contentsRect().height() - (border * 2) +1);
    m_scaledPixmap = m_pixmap.scaled(newsize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void ContactImage::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *option,
                   QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->setRenderHint(QPainter::Antialiasing);

    bg.setAlphaF(.3);
    fg.setAlphaF(.2);
    painter->setBrush(bg);
    painter->setPen(fg);
    painter->translate(.5, .5); // unblur (align to full pixel)

    // compute rect of the pixmap to paint the frame
    QRectF r = QRect(contentsRect().left(), contentsRect().top(), m_scaledPixmap.width() + border*2, m_scaledPixmap.height() + border*2);
    painter->drawRoundedRect(r, border*2, border*2);

    // paint our cached scaled version of the pixmap on top of that
    painter->drawPixmap(QPoint(border, border), m_scaledPixmap);

}

void ContactImage::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    // resize m_image;
    pixmapUpdated();
}

#include "contactimage.moc"
