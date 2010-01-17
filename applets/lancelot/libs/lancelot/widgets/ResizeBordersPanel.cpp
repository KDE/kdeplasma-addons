/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "ResizeBordersPanel.h"
#include "Widget.h"

#define COLOR 255,0,0

// TODO: Make ResizeBordersPanel use FullBorderLayout
// TODO: Make the resize feedback themable

namespace Lancelot
{

class ResizeBordersPanel::Private {
public:
    class BorderItem: public Widget {
    public:
        BorderItem(QGraphicsItem * parent = 0, ResizeBordersPanel::Private * d = 0, int position = 0)
            : Widget(parent), m_position(position), m_d(d)
        {
            setAcceptsHoverEvents(true);
        }

    protected:
        void hoverEnterEvent(QGraphicsSceneHoverEvent * event)
        {
            Widget::hoverEnterEvent(event);
            m_d->activePosition = m_position;
            m_d->updateChildren();
        }

        void hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
        {
            Widget::hoverLeaveEvent(event);
            m_d->activePosition = 0;
            m_d->updateChildren();
        }

        int m_position;
        ResizeBordersPanel::Private * m_d;
    };

    void add(int where)
    {
        BorderItem * item = new BorderItem(q, this, where);
        borderItems[where] = item;

        // Setting the cursor
        if (where == Left || where == Right) {
            item->setCursor(Qt::SizeHorCursor);
        } else if (where == Top || where == Bottom) {
            item->setCursor(Qt::SizeVerCursor);
        } else if (where == (Left | Bottom) || where == (Right | Top)) {
            item->setCursor(Qt::SizeBDiagCursor);
        } else if (where == (Left | Top)    || where == (Right | Bottom)) {
            item->setCursor(Qt::SizeFDiagCursor);
        }
    }

    Private(ResizeBordersPanel * parent)
        : activePosition(0), q(parent)
    {
        add(Left);
        add(Right);
        add(Top);
        add(Bottom);

        add(Left  | Top);
        add(Left  | Bottom);
        add(Right | Top);
        add(Right | Bottom);
    }

    enum Edge {
        Left = 1,
        Right = 2,
        Top = 4,
        Bottom = 8
    };

    void updateChildren()
    {
        foreach (QGraphicsWidget * item, borderItems) {
            item->update();
        }
    }

    QMap < int, QGraphicsWidget * > borderItems;
    int activePosition;
    ResizeBordersPanel * q;
};

ResizeBordersPanel::ResizeBordersPanel(QGraphicsItem * parent)
    : Panel(parent), d(new Private(this))
{
}

ResizeBordersPanel::~ResizeBordersPanel()
{
    delete d;
}

void ResizeBordersPanel::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    Panel::resizeEvent(event);

    qreal left   = borderSize(Plasma::LeftMargin);
    qreal right  = borderSize(Plasma::RightMargin);
    qreal top    = borderSize(Plasma::TopMargin);
    qreal bottom = borderSize(Plasma::BottomMargin);

    QRectF g;

    // Top edge
    g.setBottomRight(QPointF(left, top));
    d->borderItems[Private::Left | Private::Top]->setGeometry(g);

    g.setLeft(g.right());
    g.setWidth(geometry().width() - left - right);
    d->borderItems[Private::Top]->setGeometry(g);

    g.setLeft(g.right());
    g.setWidth(right);
    d->borderItems[Private::Right | Private::Top]->setGeometry(g);

    // Middle items
    g = QRectF();
    g.setTop(top);
    g.setHeight(geometry().height() - top - bottom);

    g.setWidth(left);
    d->borderItems[Private::Left]->setGeometry(g);

    g.setWidth(right);
    g.moveRight(geometry().width());
    d->borderItems[Private::Right]->setGeometry(g);

    // Bottom edge
    g = QRectF();
    g.setHeight(bottom);
    g.moveBottom(geometry().height());

    g.setWidth(left);
    d->borderItems[Private::Left | Private::Bottom]->setGeometry(g);

    g.setLeft(g.right());
    g.setWidth(geometry().width() - left - right);
    d->borderItems[Private::Bottom]->setGeometry(g);

    g.setLeft(g.right());
    g.setWidth(right);
    d->borderItems[Private::Right | Private::Bottom]->setGeometry(g);
}

} // namespace Lancelot

