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

#include "contactimage.h"

//Qt
#include <QGraphicsWidget>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

// KDE
#include <KIcon>

// Plasma
#include <Plasma/DataEngine>
#include <Plasma/Theme>


using namespace Plasma;

ContactImage::ContactImage(DataEngine* engine, QGraphicsWidget* parent)
    : QGraphicsWidget(parent), m_engine(engine)
{
    border = 1; // should be a power of two, otherwise we get blurry lines
    fg = Theme::defaultTheme()->color(Theme::TextColor);
    bg = Theme::defaultTheme()->color(Theme::BackgroundColor);
    pixmapUpdated();
}


void ContactImage::setUrl(const QUrl& url)
{
    if (!m_source.isEmpty()) {
        m_engine->disconnectSource(m_source, this);
    }
    m_source = url.isValid() ? "Pixmap\\url:" + url.toString() : QString();
    if (!m_source.isEmpty()) {
        m_engine->connectSource(m_source, this);
    }
}


void ContactImage::dataUpdated(const QString& source, const DataEngine::Data& data)
{
    Q_UNUSED(source)
    m_pixmap = data.value("Pixmap").value<QPixmap>();
    pixmapUpdated();
    update();
}


void ContactImage::pixmapUpdated()
{
    QSize newSize = QSize(contentsRect().width() - (border * 2), contentsRect().height() - (border * 2));
    if (!m_pixmap.isNull()) {
        if (newSize.width() > m_pixmap.width()) {
            newSize.setWidth(m_pixmap.width());
        }
        if (newSize.height() > m_pixmap.height()) {
            newSize.setHeight(m_pixmap.height());
        }
        m_scaledPixmap = m_pixmap.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        m_scaledPixmap = KIcon("system-users").pixmap(newSize);
    }
}


void ContactImage::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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


void ContactImage::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    Q_UNUSED(event)

    pixmapUpdated();
}


#include "contactimage.moc"
