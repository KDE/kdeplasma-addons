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

#include "ScrollBar.h"
#include "ExtenderButton.h"

namespace Lancelot
{

class ScrollBar::Private {
public:
    Private(ScrollBar * parent)
      : minimum(0),
        maximum(100),
        value(0),
        viewSize(10),
        orientation(Qt::Vertical),
        q(parent)
    {
        upButton   = new ExtenderButton("", "+", "", parent);
        downButton = new ExtenderButton("", "+", "", parent);
        upBar      = new ExtenderButton("", "+", "", parent);
        downBar    = new ExtenderButton("", "+", "", parent);
        handle     = new ExtenderButton("", "+", "", parent);
        upButton->setGroupByName("SystemButtons");
        downButton->setGroupByName("SystemButtons");
        handle->setGroupByName("SystemButtons");
    }

    /** Flips the source rect if the orientation is Horizontal */
    QRectF orientateRect(const QRectF & rect) {
        if (orientation == Qt::Vertical) {
            return rect;
        }

        QRectF result;
        result.setLeft(rect.top());
        result.setTop(rect.left());
        result.setWidth(rect.height());
        result.setHeight(rect.width());
        return result;
    }

    void invalidate()
    {
        QRectF geometry = orientateRect(q->geometry());

        QRectF itemRect = geometry;

        itemRect.setHeight(itemRect.width());
        upButton->setGeometry(orientateRect(itemRect));

        itemRect.moveBottom(geometry.bottom());
        downButton->setGeometry(orientateRect(itemRect));

        positionScroll();
    }

    void positionScroll()
    {
        QRectF geometry = orientateRect(q->geometry());
        geometry.setTop(geometry.top() + geometry.width());
        geometry.setBottom(geometry.bottom() - geometry.width());

        if (minimum >= maximum) {
            handle->setGeometry(geometry);
            upBar->setGeometry(QRectF());
            downBar->setGeometry(QRectF());
            return;
        }

        qreal diff;

        qreal handleSize = geometry.height() * (viewSize / (qreal)(maximum - minimum));
        diff = 2 * geometry.width();
        if (handleSize < diff) {
            handleSize = diff;
        }

        diff = geometry.height() - handleSize;

        QRectF itemRect = geometry;

        // up bar
        itemRect.setHeight(
                diff * ((value - minimum) / (qreal)(maximum - minimum))
                );
        upBar->setGeometry(itemRect);

        itemRect.setTop(itemRect.bottom());
        itemRect.setHeight(handleSize);
        handle->setGeometry(itemRect);

        itemRect.setTop(itemRect.bottom());
        itemRect.setBottom(geometry.bottom());
        downBar->setGeometry(itemRect);
    }

    int minimum;
    int maximum;
    int value;
    int viewSize;
    Qt::Orientation orientation;

    ExtenderButton * upButton;
    ExtenderButton * downButton;
    ExtenderButton * upBar;
    ExtenderButton * downBar;
    ExtenderButton * handle;

    ScrollBar * q;
};

ScrollBar::ScrollBar(QString name, QGraphicsItem * parent)
  : Widget(name, parent), d(new Private(this))
{
}

ScrollBar::~ScrollBar()
{
    delete d;
}

void ScrollBar::setMinimum(int value)
{
    if (d->minimum == value) {
        return;
    }

    d->minimum = value;
    if (d->value < d->minimum) {
        d->value = d->minimum;
    }
    if (d->maximum < d->minimum) {
        d->maximum = d->minimum;
    }
    d->positionScroll();
}

int ScrollBar::minimum() const
{
    return d->minimum;
}

void ScrollBar::setMaximum(int value)
{
    if (d->maximum == value) {
        return;
    }

    d->maximum = value;
    if (d->value > d->maximum) {
        d->value = d->maximum;
    }
    if (d->maximum < d->minimum) {
        d->minimum = d->maximum;
    }
    d->positionScroll();
}

int ScrollBar::maximum() const
{
    return d->maximum;
}

void ScrollBar::setValue(int value)
{
    if (value < d->minimum) {
        value = d->minimum;
    } else if (value > d->maximum) {
        value = d->maximum;
    }

    if (d->value == value) {
        return;
    }

    d->value = value;
    d->positionScroll();
    emit valueChanged(d->value);
}

int ScrollBar::value() const
{
    return d->value;
}

void ScrollBar::setViewSize(int value)
{
    if (d->viewSize = value) {
        return;
    }

    d->viewSize = value;
    if (d->viewSize > d->maximum - d->minimum) {
        d->viewSize = d->maximum - d->minimum;
    }

    d->positionScroll();
}

int ScrollBar::viewSize() const
{
    return d->viewSize;
}

void ScrollBar::setOrientation(Qt::Orientation value)
{
    d->orientation = value;
    d->invalidate();
}

Qt::Orientation ScrollBar::orientation() const
{
    return d->orientation;
}

void ScrollBar::setGeometry(qreal x, qreal y, qreal w, qreal h)
{
    setGeometry(QRectF(x, y, w, h));
}

void ScrollBar::setGeometry(const QRectF & geometry)
{
    Widget::setGeometry(geometry);
    d->invalidate();
}

void ScrollBar::setGroup(WidgetGroup * g)
{
    Widget::setGroup(g);
    //d->titleWidget.setGroupByName(group()->name() + "-Title");
}

} // namespace Lancelot

// #include "ScrollBar.moc"

