/*
 * Copyright 2009  Alex Merry <alex.merry@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "albumart.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneResizeEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <KDebug>

#include <Plasma/Svg>

AlbumArt::AlbumArt(QGraphicsWidget* parent)
    : QGraphicsWidget(parent),
      m_noAlbumSvg(0)
{
    QSizePolicy policy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    //policy.setHeightForWidth(true);
    setSizePolicy(policy);
}

AlbumArt::~AlbumArt()
{
}

void AlbumArt::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    if (m_pixmap.isNull()) {
        if (m_noAlbumSvg) {
            QSize s = m_noAlbumSvg->size();
            QRectF r(QPointF(0, 0), s);
            r.moveTo(rect().center().x() - (r.width() / 2), r.y());
            m_noAlbumSvg->paint(painter, r);
        }
        return;
    }

    if (m_scaledPixmap.size() != size()) {
        QSize scaledSize (m_pixmap.size());
        scaledSize.scale (size().toSize(), Qt::KeepAspectRatio);

        if (m_scaledPixmap.size() != scaledSize) {
            // Faster smooth transformation if scaledSize is noticeably smaller than m_pixmap
            m_scaledPixmap = m_pixmap.scaled(2 * scaledSize)
                                     .scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
    }

    painter->drawPixmap(0, 0, m_scaledPixmap);
}

void AlbumArt::setPixmap(const QPixmap& pixmap)
{
    m_pixmap = pixmap;
    m_scaledPixmap = QPixmap();
    if (pixmap.isNull()) {
        if (!m_noAlbumSvg) {
            m_noAlbumSvg = new Plasma::Svg(this);
            m_noAlbumSvg->setImagePath("widgets/nowplaying/nocover");
            QSizeF s = size();
            if (s.width() > s.height()) {
                s.setWidth(s.height());
            } else if (s.height() > s.width()) {
                s.setHeight(s.width());
            }
            m_noAlbumSvg->resize(s);
        }
    } else {
        delete m_noAlbumSvg;
        m_noAlbumSvg = 0;
    }

    show();
    updateGeometry();
    update();
}

void AlbumArt::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);

    // cast to QSize for rounding effects - there's no point being too
    // dogmatic about flushing the sizehint
    QSize oldSize = event->oldSize().toSize();
    QSize newSize = event->newSize().toSize();
    if ((oldSize.height() != newSize.height()) &&
        (newSize.height() != newSize.width())) {
        //kDebug() << "Old size:" << oldSize << "; new size:" << newSize;
        updateGeometry();
        if (m_noAlbumSvg) {
            QSizeF s = size();
            if (s.width() > s.height()) {
                s.setWidth(s.height());
            } else if (s.height() > s.width()) {
                s.setHeight(s.width());
            }
            m_noAlbumSvg->resize(s);
        }
    }
}

QSizeF AlbumArt::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    QSizeF hint;
    if (which == Qt::PreferredSize) {
        if (m_pixmap.isNull()) {
            if (m_noAlbumSvg) {
                hint = QGraphicsWidget::sizeHint(which, constraint).expandedTo(QSizeF(128, 128));
            } else {
                hint = QSize(0, 0);
            }
        } else if (size().height() < 1) {
            hint = m_pixmap.size();
        } else {
            hint = QSizeF(size().height(), size().height());
        }
    } else {
        hint = QGraphicsWidget::sizeHint(which, constraint);
    }

    if (which == Qt::MinimumSize) {
        hint.setHeight(0);
    }

    //kDebug() << "Size hint for mode" << which << "is" << hint;

    return hint;
}

// vim: sw=4 sts=4 et tw=100
