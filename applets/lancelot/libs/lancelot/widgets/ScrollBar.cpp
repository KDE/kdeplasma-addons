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
#include <KDebug>

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
        stepSize(5),
        pageSize(10),
        orientation(Qt::Vertical),
        activationMethod(ExtenderActivate),
        q(parent)
    {
        upButton   = new ExtenderButton("", "", parent);
        downButton = new ExtenderButton("", "", parent);
        upBar      = new ExtenderButton("", "", parent);
        downBar    = new ExtenderButton("", "", parent);
        handle     = new ExtenderButton("", "", parent);

        kDebug() << "Connecting everything";
        kDebug() << connect(upBar, SIGNAL(clicked()), parent, SLOT(pageDecrease()));
        kDebug() << connect(downBar, SIGNAL(clicked()), parent, SLOT(pageIncrease()));

        upTimer.setInterval(100);
        upTimer.setSingleShot(false);
        kDebug() << connect(&upTimer, SIGNAL(timeout()), parent, SLOT(stepDecrease()));

        downTimer.setInterval(100);
        downTimer.setSingleShot(false);
        kDebug() << connect(&downTimer, SIGNAL(timeout()), parent, SLOT(stepIncrease()));
    }

    ~Private()
    {
        delete upButton;
        delete upBar;
        delete downButton;
        delete downBar;
        delete handle;
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
        geometry.moveTopLeft(QPointF(0, 0));

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
        geometry.moveTopLeft(QPointF(0, 0));

        geometry.setTop(geometry.top() + geometry.width());
        geometry.setBottom(geometry.bottom() - geometry.width());

        if (minimum >= maximum) {
            handle->setGeometry(orientateRect(geometry));
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
        upBar->setGeometry(orientateRect(itemRect));

        itemRect.setTop(itemRect.bottom());
        itemRect.setHeight(handleSize);
        handle->setGeometry(orientateRect(itemRect));

        itemRect.setTop(itemRect.bottom());
        itemRect.setBottom(geometry.bottom());
        downBar->setGeometry(orientateRect(itemRect));
    }

    int minimum;
    int maximum;
    int value;
    int viewSize;
    int stepSize;
    int pageSize;

    Qt::Orientation orientation;
    ActivationMethod activationMethod;

    ExtenderButton * upButton;
    ExtenderButton * downButton;
    ExtenderButton * upBar;
    ExtenderButton * downBar;
    ExtenderButton * handle;

    QTimer downTimer, upTimer;

    ScrollBar * q;
};

ScrollBar::ScrollBar(QGraphicsItem * parent)
  : Widget(parent), d(new Private(this))
{
    setGroupByName("ScrollBar");

    // Initially, the method is set to ExtenderActivate
    // so the following method will not just exit,
    // but will set the necessary connections.
    setActivationMethod(HoverActivate);
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
    if (d->viewSize == value) {
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

void ScrollBar::setStepSize(int value)
{
    d->stepSize = value;
}

int ScrollBar::stepSize() const
{
    return d->stepSize;
}

void ScrollBar::setPageSize(int value)
{
    d->pageSize = value;
}

int ScrollBar::pageSize() const
{
    return d->pageSize;
}

void ScrollBar::setOrientation(Qt::Orientation value)
{
    if (d->orientation == value) {
        return;
    }
    d->orientation = value;
    setGroup(group());
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
    kDebug() << group()->name();
    switch (d->orientation) {
        case Qt::Vertical:
            d->upButton->setGroupByName(group()->name() + "-UpButton");
            d->downButton->setGroupByName(group()->name() + "-DownButton");
            d->upBar->setGroupByName(group()->name() + "-UpBar");
            d->downBar->setGroupByName(group()->name() + "-DownBar");
            d->handle->setGroupByName(group()->name() + "-HandleVertical");
            break;
        case Qt::Horizontal:
            d->upButton->setGroupByName(group()->name() + "-LeftButton");
            d->downButton->setGroupByName(group()->name() + "-RightButton");
            d->upBar->setGroupByName(group()->name() + "-LeftBar");
            d->downBar->setGroupByName(group()->name() + "-RightBar");
            d->handle->setGroupByName(group()->name() + "-HandleHorizontal");
            break;
    }
}

void ScrollBar::stepIncrease()
{
    setValue(value() + d->stepSize);
}

void ScrollBar::stepDecrease()
{
    setValue(value() - d->stepSize);
}

void ScrollBar::pageIncrease()
{
    setValue(value() + d->pageSize);
}

void ScrollBar::pageDecrease()
{
    setValue(value() - d->pageSize);
}

void ScrollBar::setActivationMethod(ActivationMethod method)
{
    if (method == ExtenderActivate) {
        method = HoverActivate;
    }

    if (method == d->activationMethod) {
        return;
    }

    disconnect(d->upButton, 0, &(d->upTimer), 0);
    disconnect(d->downButton, 0, &(d->downTimer), 0);

    switch (method) {
        case HoverActivate:
            d->activationMethod = HoverActivate;
            connect(
                d->upButton, SIGNAL(mouseHoverEnter()),
                &(d->upTimer), SLOT(start())
                );
            connect(
                d->upButton, SIGNAL(mouseHoverLeave()),
                &(d->upTimer), SLOT(stop())
                );
            connect(
                d->downButton, SIGNAL(mouseHoverEnter()),
                &(d->downTimer), SLOT(start())
                );
            connect(
                d->downButton, SIGNAL(mouseHoverLeave()),
                &(d->downTimer), SLOT(stop())
                );
            break;
        case ClickActivate:
            d->activationMethod = ClickActivate;
            connect(
                d->upButton, SIGNAL(pressed()),
                &(d->upTimer), SLOT(start())
                );
            connect(
                d->upButton, SIGNAL(released()),
                &(d->upTimer), SLOT(stop())
                );
            connect(
                d->downButton, SIGNAL(pressed()),
                &(d->downTimer), SLOT(start())
                );
            connect(
                d->downButton, SIGNAL(released()),
                &(d->downTimer), SLOT(stop())
                );
            break;
    }
}

ActivationMethod ScrollBar::activationMethod() const
{
    return d->activationMethod;
}

void ScrollBar::groupUpdated()
{
    Widget::groupUpdated();
    if (group()->hasProperty("ActivationMethod")) {
        setActivationMethod((ActivationMethod)(group()->property("ActivationMethod").toInt()));
    }
}

} // namespace Lancelot

// #include "ScrollBar.moc"

