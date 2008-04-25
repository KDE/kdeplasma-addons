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

#include <KDebug>

#include "ScrollPane.h"
#include "Widget.h"


namespace Lancelot
{

// Scrollable interface common functions implementation

class Scrollable::Private {
public:
    ScrollPane * pane;
};

Scrollable::Scrollable()
  : d(new Private())
{
    d->pane = NULL;
}

Scrollable::~Scrollable()
{
    delete d;
}

void Scrollable::setScrollPane(ScrollPane * pane)
{
    if (pane == d->pane) return;
    d->pane = pane;
    d->pane->setScrollableWidget(this);
}

ScrollPane * Scrollable::scrollPane() const
{
    return d->pane;
}

// ScrollPane implementation

class ScrollButton: public BasicWidget {
public:
    ScrollButton(QString name, Qt::ArrowType direction, ScrollPane * parent)
        : BasicWidget(name, parent)
    {
        QString directionSuffix;
        switch (direction) {
            case Qt::LeftArrow:
                directionSuffix = "left";
                break;
            case Qt::RightArrow:
                directionSuffix = "right";
                break;
            case Qt::UpArrow:
                directionSuffix = "up";
                break;
            case Qt::DownArrow:
                directionSuffix = "down";
                break;
            case Qt::NoArrow:
                directionSuffix = "no";
                break;
        }
        setIconInSvg(new Plasma::Svg("arrow-" + directionSuffix));

        pane = parent;
    }
private:
    ScrollPane * pane;
};

class ScrollPane::Private {
public:
    Scrollable   * widget;
    ScrollButton * up;
    ScrollButton * down;
    ScrollButton * left;
    ScrollButton * right;
};

ScrollPane::ScrollPane(QString name, QGraphicsItem * parent)
    : Widget(name, parent), d(new Private())
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape);

    up     = new ScrollButton(Qt::UpArrow);
    down   = new ScrollButton(Qt::DownArrow);
    left   = new ScrollButton(Qt::LeftArrow);
    right  = new ScrollButton(Qt::RightArrow);

}

ScrollPane::~ScrollPane()
{
    delete d;
}

void ScrollPane::setGeometry(qreal x, qreal y, qreal w, qreal h)
{
    setGeometry(QRectF(x, y, w, h));
}

void ScrollPane::setGeometry(const QRectF & geometry)
{
    Widget::setGeometry(geometry);
}

void ScrollPane::setScrollableWidget(Scrollable * widget)
{
    d->widget = widget;
}

QSizeF ScrollPane::maximumViewportSize() const
{
    return size();
}

QSizeF ScrollPane::currentViewportSize() const
{
    return size();
}

} // namespace Lancelot

