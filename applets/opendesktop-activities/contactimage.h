/***************************************************************************
 *   Copyright 2009 by Sebastian Kügler <sebas@kde.org>                    *
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

#ifndef CONTACTIMAGE_H
#define CONTACTIMAGE_H

//Qt
#include <QGraphicsWidget>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

class ContactImage : public QGraphicsWidget
{
    Q_OBJECT

    public:
        ContactImage(QGraphicsWidget *parent = 0);
        virtual ~ContactImage();
        void setImage(const QImage &image);
        void setPixmap(const QPixmap &pixmap);

        QColor fg;
        QColor bg;
        //QImage image;
        int border;

    protected:
        void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

        void resizeEvent(QGraphicsSceneResizeEvent *event);

    private:
        void pixmapUpdated();
        QPixmap m_pixmap;
        QPixmap m_scaledPixmap;
};

#endif

