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
#ifndef ALBUMART_H
#define ALBUMART_H

#include <QGraphicsWidget>
#include <QPixmap>

#include "nowplaying.h"

class QGraphicsPixmapItem;
class QPixmap;

class AlbumArt : public QGraphicsWidget
{
    Q_OBJECT

public:
    AlbumArt(QGraphicsWidget* parent = 0);
    ~AlbumArt();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

public slots:
    void setPixmap(const QPixmap& pixmap);

protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
    void resizeEvent(QGraphicsSceneResizeEvent* event);

private:
    QPixmap m_pixmap;
    QPixmap m_scaledPixmap;
};

#endif // ALBUMART_H
