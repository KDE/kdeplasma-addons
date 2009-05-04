#ifndef SINGLE_FEED_H
#define SINGLE_FEED_H
/*
 *   Copyright (C) 2007 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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


#include "feeddata.h"
#include <QGraphicsItem>
#include <QGraphicsWidget>

class QRect;

namespace Plasma {
    class Svg;
}

class SingleFeedItem : public QGraphicsWidget
{
    public:
        SingleFeedItem(QGraphicsItem * parent = 0);
        ~SingleFeedItem();
        void setFeedData(FeedData feeddata);
        FeedData feedData() const;
        void setRect(const QRect& rect);
        int itemNumber() const;
        void setDisplayExtra(bool extra);
        bool displayExtra() const;

        virtual QRectF boundingRect() const;
        virtual void paint(QPainter *p,
                           const QStyleOptionGraphicsItem *option,
                           QWidget *widget = 0);
    private:
        QRect m_rect;
        FeedData m_feeditem;
        Plasma::Svg * m_background;
        QGraphicsTextItem * m_html;
        bool m_displayExtra;
       // QImage m_htmlImg;
};

#endif // SINGLE_FEED_H
