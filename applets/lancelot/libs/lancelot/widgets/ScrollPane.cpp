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
#include "ScrollBar.h"
#include "Widget.h"
#include <lancelot/layouts/FullBorderLayout.h>

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

class ScrollPane::Private {
public:
    Private(ScrollPane * parent)
        : q(parent), widget(NULL), layout(NULL),
          vertical(NULL), horizontal(NULL),
          flags(ScrollPane::ClipScrollable)
    {
    }

    void updateViewport()
    {
        kDebug() << QPointF(horizontal->value(), vertical->value())
                 << q->currentViewportSize() << q->maximumViewportSize();
        widget->viewportChanged(QRectF(QPointF(horizontal->value(), vertical->value()),
                q->currentViewportSize()));
    }

    ScrollPane * q;
    Scrollable * widget;
    FullBorderLayout * layout;
    ScrollBar * vertical;
    ScrollBar * horizontal;
    QGraphicsWidget * centerContainer;
    Flags flags;
};

ScrollPane::ScrollPane(QGraphicsItem * parent)
    : Widget(parent), d(new Private(this))
{
    setAcceptsHoverEvents(true);
    d->layout = new FullBorderLayout(this);

    d->vertical   = new ScrollBar(this);
    d->vertical->setOrientation(Qt::Vertical);
    d->vertical->setZValue(1);
    d->horizontal = new ScrollBar(this);
    d->horizontal->setOrientation(Qt::Horizontal);
    d->horizontal->setZValue(1);

    d->centerContainer = new QGraphicsWidget(this);
    d->centerContainer->setAcceptsHoverEvents(true);
    d->centerContainer->setFlag(QGraphicsItem::ItemClipsChildrenToShape);

    connect (d->vertical, SIGNAL(valueChanged(int)),
            this, SLOT(scrollVertical(int)));
    connect (d->horizontal, SIGNAL(valueChanged(int)),
            this, SLOT(scrollHorizontal(int)));

    d->layout->addItem(d->vertical, FullBorderLayout::Right);
    d->layout->addItem(d->horizontal, FullBorderLayout::Bottom);
    d->layout->addItem(d->centerContainer, FullBorderLayout::Center);

    d->layout->setContentsMargins(0, 0, 0, 0);

    setLayout(d->layout);
    L_WIDGET_SET_INITIALIZED;
}

ScrollPane::~ScrollPane()
{
    delete d->layout;
    delete d->horizontal;
    delete d->vertical;
    delete d->centerContainer;
    delete d;
}

void ScrollPane::setScrollableWidget(Scrollable * widget)
{
    if (d->widget == widget) {
        return;
    }

    d->widget = widget;

    QGraphicsWidget * qgw = dynamic_cast<QGraphicsWidget *>(widget);
    if (qgw) {
        qgw->setParentItem(d->centerContainer);
        scrollableWidgetSizeUpdated();
        d->updateViewport();
    }
}

QSizeF ScrollPane::maximumViewportSize() const
{
    return size();
}

QSizeF ScrollPane::currentViewportSize() const
{
    return d->centerContainer->size();
}

void ScrollPane::scrollableWidgetSizeUpdated()
{
    if (!d->widget) {
        return;
    }

    d->layout->setAutoSize(FullBorderLayout::RightBorder);
    d->layout->setAutoSize(FullBorderLayout::BottomBorder);

    bool hasHorizontal;
    bool hasVertical;

    hasHorizontal = d->widget->fullSize().width() > maximumViewportSize().width();
    if (hasVertical   = d->widget->fullSize().height() > maximumViewportSize().height()) {
        hasHorizontal = d->widget->fullSize().width() > currentViewportSize().width();
    }

    d->horizontal->setVisible(hasHorizontal);
    d->vertical->setVisible(hasVertical);

    if (!hasHorizontal) {
        d->horizontal->setValue(0);
        d->layout->setSize(0, FullBorderLayout::BottomBorder);
    }
    if (!hasVertical) {
        d->vertical->setValue(0);
        d->layout->setSize(0, FullBorderLayout::RightBorder);
    }

    int viewportSize;

    if (hasHorizontal) {
        viewportSize = currentViewportSize().width();
        d->horizontal->setMinimum(0);
        d->horizontal->setMaximum(d->widget->fullSize().width() - viewportSize);
        d->horizontal->setViewSize(viewportSize);
        d->horizontal->setPageSize(viewportSize);
        d->horizontal->setStepSize(d->widget->scrollUnit(Qt::Horizontal));
        if ((d->flags & HoverShowScrollbars) && !isHovered()) {
            d->horizontal->hide();
        }
    }

    if (hasVertical) {
        viewportSize = currentViewportSize().height();
        d->vertical->setMinimum(0);
        d->vertical->setMaximum(d->widget->fullSize().height() - viewportSize);
        d->vertical->setViewSize(viewportSize);
        d->vertical->setPageSize(viewportSize);
        d->vertical->setStepSize(d->widget->scrollUnit(Qt::Vertical));
        if ((d->flags & HoverShowScrollbars) && !isHovered()) {
            d->vertical->hide();
        }
    }
}

void ScrollPane::setGeometry(const QRectF & rect)
{
    if (geometry() == rect) {
        return;
    }
    kDebug() << rect;
    Widget::setGeometry(rect);
    kDebug() << geometry();
    scrollableWidgetSizeUpdated();
    d->updateViewport();
}

void ScrollPane::scrollHorizontal(int value)
{
    kDebug() << value;
    d->horizontal->setValue(value);
    d->updateViewport();
}

void ScrollPane::scrollVertical(int value)
{
    kDebug() << value;
    d->vertical->setValue(value);
    d->updateViewport();
}

void ScrollPane::setFlag(Flag flag)
{
    d->flags |= flag;
    setFlags(d->flags);
}

void ScrollPane::clearFlag(Flag flag)
{
    d->flags &= ~ flag;
    setFlags(d->flags);
}

ScrollPane::Flags ScrollPane::flags() const
{
    return d->flags;
}

void ScrollPane::setFlags(Flags flags)
{
    d->flags = flags;
    if (d->flags & ClipScrollable) {
        d->centerContainer->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
    } else {
        d->centerContainer->setFlags(
                d->centerContainer->flags() & ~QGraphicsItem::ItemClipsChildrenToShape);
    }

    if ((d->flags & HoverShowScrollbars) && !isHovered()) {
        d->horizontal->hide();
        d->vertical->hide();
    }
}

void ScrollPane::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    kDebug();
    Widget::hoverEnterEvent(event);

    if (!(d->flags & HoverShowScrollbars)) {
        return;
    }
    if (d->layout->size(FullBorderLayout::RightBorder) != 0) {
        d->vertical->setVisible(true);
    }
    if (d->layout->size(FullBorderLayout::BottomBorder) != 0) {
        d->horizontal->setVisible(true);
    }
}

void ScrollPane::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    kDebug();
    Widget::hoverLeaveEvent(event);

    if (!(d->flags & HoverShowScrollbars)) {
        return;
    }

    if (d->layout->size(FullBorderLayout::RightBorder) != 0) {
        d->vertical->hide();
    }
    if (d->layout->size(FullBorderLayout::BottomBorder) != 0) {
        d->horizontal->hide();
    }
}

} // namespace Lancelot

