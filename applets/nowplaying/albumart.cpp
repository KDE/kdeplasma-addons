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

#include <kdebug.h>

AlbumArt::AlbumArt(QGraphicsWidget* parent)
    : QGraphicsWidget(parent)
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

    if (m_pixmap.isNull())
        return;

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
        hide();
        updateGeometry();
    } else {
        show();
        updateGeometry();
    }
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
    }
}

QSizeF AlbumArt::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    QSizeF hint;
    if (which == Qt::PreferredSize) {
        if (m_pixmap.isNull()) {
            hint = QSize(0, 0);
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
