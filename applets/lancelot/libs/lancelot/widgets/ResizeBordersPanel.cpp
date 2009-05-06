/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

        /*
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0)
        {
            Q_UNUSED(widget);
            if (isHovered()) {
                if (m_position == ResizeBordersPanel::Private::Top
                        || m_position == ResizeBordersPanel::Private::Bottom) {
                    QLinearGradient fade(0, 0, option->rect.width() / 2, 0);
                    fade.setSpread(QGradient::ReflectSpread);
                    fade.setColorAt(0, QColor(COLOR, 0));
                    fade.setColorAt(1, QColor(COLOR, 200));
                    painter->fillRect(option->rect, fade);
                } else if (m_position == ResizeBordersPanel::Private::Left
                        || m_position == ResizeBordersPanel::Private::Right) {
                    QLinearGradient fade(0, 0, 0, option->rect.height() / 2);
                    fade.setSpread(QGradient::ReflectSpread);
                    fade.setColorAt(0, QColor(COLOR, 0));
                    fade.setColorAt(1, QColor(COLOR, 200));
                    painter->fillRect(option->rect, fade);
                } else {
                    painter->fillRect(option->rect, QBrush(QColor(COLOR, 200)));
                }
            } else {
                if ((m_position & m_d->activePosition) && (m_d->activePosition > m_position)) {
                    if ((m_position == Left) || (m_position == Right)) {
                        if (m_d->activePosition & Top) {
                            QLinearGradient fade(0, 0, 0, option->rect.height() / 2);
                            fade.setColorAt(0, QColor(COLOR, 200));
                            fade.setColorAt(1, QColor(COLOR, 0));
                            painter->fillRect(option->rect, fade);
                        } else {
                            QLinearGradient fade(0, option->rect.height() / 2, 0, option->rect.height());
                            fade.setColorAt(1, QColor(COLOR, 200));
                            fade.setColorAt(0, QColor(COLOR, 0));
                            painter->fillRect(option->rect, fade);
                        }
                    } else if ((m_position == Top) || (m_position == Bottom)) {
                        if (m_d->activePosition & Left) {
                            QLinearGradient fade(0, 0, option->rect.width() / 2, 0);
                            fade.setColorAt(0, QColor(COLOR, 200));
                            fade.setColorAt(1, QColor(COLOR, 0));
                            painter->fillRect(option->rect, fade);
                        } else {
                            QLinearGradient fade(option->rect.width() / 2, 0, option->rect.width(), 0);
                            fade.setColorAt(1, QColor(COLOR, 200));
                            fade.setColorAt(0, QColor(COLOR, 0));
                            painter->fillRect(option->rect, fade);
                        }
                    }
                }
            }
        } */

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
    L_WIDGET_SET_INITIALIZED;
}

ResizeBordersPanel::~ResizeBordersPanel()
{
    delete d;
}

void ResizeBordersPanel::setGeometry(const QRectF & geometry)
{
    Panel::setGeometry(geometry);

    qreal left   = borderSize(Plasma::LeftMargin);
    qreal right  = borderSize(Plasma::RightMargin);
    qreal top    = borderSize(Plasma::TopMargin);
    qreal bottom = borderSize(Plasma::BottomMargin);

    QRectF g;

    // Top edge
    g.setBottomRight(QPointF(left, top));
    d->borderItems[Private::Left | Private::Top]->setGeometry(g);

    g.setLeft(g.right());
    g.setWidth(geometry.width() - left - right);
    d->borderItems[Private::Top]->setGeometry(g);

    g.setLeft(g.right());
    g.setWidth(right);
    d->borderItems[Private::Right | Private::Top]->setGeometry(g);

    // Middle items
    g = QRectF();
    g.setTop(top);
    g.setHeight(geometry.height() - top - bottom);

    g.setWidth(left);
    d->borderItems[Private::Left]->setGeometry(g);

    g.setWidth(right);
    g.moveRight(geometry.width());
    d->borderItems[Private::Right]->setGeometry(g);

    // Bottom edge
    g = QRectF();
    g.setHeight(bottom);
    g.moveBottom(geometry.height());

    g.setWidth(left);
    d->borderItems[Private::Left | Private::Bottom]->setGeometry(g);

    g.setLeft(g.right());
    g.setWidth(geometry.width() - left - right);
    d->borderItems[Private::Bottom]->setGeometry(g);

    g.setLeft(g.right());
    g.setWidth(right);
    d->borderItems[Private::Right | Private::Bottom]->setGeometry(g);


}

} // namespace Lancelot

