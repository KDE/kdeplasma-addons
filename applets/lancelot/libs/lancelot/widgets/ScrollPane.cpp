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

#include "ScrollPane.h"
#include <QGraphicsSceneWheelEvent>

#include "ScrollBar.h"
#include "Widget.h"
#include <lancelot/layouts/FullBorderLayout.h>
// #include <lancelot/layouts/FlipLayout.h>

namespace Lancelot
{

// Scrollable interface common functions implementation
//>
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
//<

// ScrollPane implementation

class ScrollPane::Private { //>
public:
    Private(ScrollPane * parent)
        : q(parent), widget(NULL), layout(NULL),
          vertical(NULL), horizontal(NULL),
          flags(ScrollPane::ClipScrollable)
    {
    }

    void updateViewport()
    {
        widget->viewportChanged(QRectF(QPointF(horizontal->value(), vertical->value()),
                q->currentViewportSize()));
    }

    ScrollPane * q;
    Scrollable * widget;
    /*FlipLayout <*/ FullBorderLayout /*>*/ * layout;
    ScrollBar * vertical;
    ScrollBar * horizontal;
    QGraphicsWidget * centerContainer;
    Flags flags;
}; //<

ScrollPane::ScrollPane(QGraphicsItem * parent) //>
    : Widget(parent), d(new Private(this))
{
    setAcceptsHoverEvents(true);
    d->layout = new /*FlipLayout <*/ FullBorderLayout /*>*/ (this);
    // d->layout->setParentLayoutItem(this);

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
} //<

ScrollPane::~ScrollPane() //>
{
    // delete d->layout;
    setLayout(NULL);

    delete d->horizontal;
    delete d->vertical;
    delete d->centerContainer;
    delete d;
} //<

void ScrollPane::setScrollableWidget(Scrollable * widget) //>
{
    if (d->widget == widget) {
        return;
    }

    d->widget = widget;
    widget->setScrollPane(this);

    QGraphicsWidget * qgw = dynamic_cast<QGraphicsWidget *>(widget);
    if (qgw) {
        qgw->setParentItem(d->centerContainer);
        scrollableWidgetSizeUpdateNeeded();
    }
} //<

QSizeF ScrollPane::maximumViewportSize() const //>
{
    return size();
} //<

QSizeF ScrollPane::currentViewportSize() const //>
{
    return d->centerContainer->size();
} //<

void ScrollPane::scrollableWidgetSizeUpdateNeeded() //>
{
    if (!d->widget || !layout()) {
        return;
    }

    d->layout->setAutoSize(FullBorderLayout::RightBorder);
    d->layout->setAutoSize(FullBorderLayout::BottomBorder);

    bool hasHorizontal;
    bool hasVertical;

    QSizeF testSize = maximumViewportSize();
    QSizeF neededSize = d->widget->sizeFor(testSize);
    hasHorizontal = neededSize.width()  > testSize.width();
    hasVertical   = neededSize.height() > testSize.height();
    if (hasVertical) {
        hasHorizontal = d->widget->sizeFor(currentViewportSize()).width()  > testSize.width();
    }

    d->horizontal->setVisible(hasHorizontal);
    d->vertical->setVisible(hasVertical);

    if (!hasHorizontal) {
        d->horizontal->setValue(0);
        d->horizontal->setRange(0, 0);
        d->layout->setSize(0, FullBorderLayout::BottomBorder);
    }

    if (!hasVertical) {
        d->vertical->setValue(0);
        d->horizontal->setRange(0, 0);
        d->layout->setSize(0, FullBorderLayout::RightBorder);
    }

    int viewportSize;

    if (hasHorizontal) {
        viewportSize = currentViewportSize().width();
        d->horizontal->setRange(0, d->widget->sizeFor(currentViewportSize()).width() - viewportSize);
        d->horizontal->setPageStep(viewportSize);
        d->horizontal->setSingleStep(d->widget->scrollUnit(Qt::Horizontal));
        if ((d->flags & HoverShowScrollbars) && !isHovered()) {
            d->horizontal->hide();
        }
    }

    if (hasVertical) {
        viewportSize = currentViewportSize().height();
        d->vertical->setRange(0, d->widget->sizeFor(currentViewportSize()).height() - viewportSize);
        d->vertical->setPageStep(viewportSize);
        d->vertical->setSingleStep(d->widget->scrollUnit(Qt::Vertical));
        if ((d->flags & HoverShowScrollbars) && !isHovered()) {
            d->vertical->hide();
        }
    }

    d->updateViewport();
} //<

void ScrollPane::resizeEvent(QGraphicsSceneResizeEvent * event) //>
{
    Lancelot::Widget::resizeEvent(event);
    scrollableWidgetSizeUpdateNeeded();
} //<

void ScrollPane::scrollHorizontal(int value) //>
{
    d->horizontal->setValue(value);
    d->updateViewport();
} //<

void ScrollPane::scrollVertical(int value) //>
{
    d->vertical->setValue(value);
    d->updateViewport();
} //<

void ScrollPane::setFlag(Flag flag) //>
{
    d->flags |= flag;
    setFlags(d->flags);
} //<

void ScrollPane::clearFlag(Flag flag) //>
{
    d->flags &= ~ flag;
    setFlags(d->flags);
} //<

ScrollPane::Flags ScrollPane::flags() const //>
{
    return d->flags;
} //<

void ScrollPane::setFlags(Flags flags) //>
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
} //<

void ScrollPane::hoverEnterEvent(QGraphicsSceneHoverEvent * event) //>
{
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
} //<

void ScrollPane::hoverLeaveEvent(QGraphicsSceneHoverEvent * event) //>
{
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
} //<

void ScrollPane::wheelEvent(QGraphicsSceneWheelEvent * event) //>
{
    if (event->modifiers() & Qt::ControlModifier) {
        d->horizontal->wheelEvent(event);
    } else {
        d->vertical->wheelEvent(event);
    }
} //<

void ScrollPane::setFlip(Plasma::Flip flip) //>
{
    if (flip & Plasma::HorizontalFlip) {
        // TODO: Replace this with a real removeItem(...)
        d->layout->addItem(NULL, FullBorderLayout::Right);
        d->layout->addItem(d->vertical, FullBorderLayout::Left);

    } else {
        d->layout->addItem(NULL, FullBorderLayout::Left);
        d->layout->addItem(d->vertical, FullBorderLayout::Right);

    }
    // d->layout->setFlip(flip);
} //<

void ScrollPane::scrollTo(QRectF rect) //>
{
    QSizeF viewportSize = currentViewportSize();
    QSizeF scrollableSize = d->widget->sizeFor(viewportSize);

    // Vertical scroll
    if (d->vertical->value() > rect.top()) {
        scrollVertical(rect.top());
    } else if (d->vertical->value() + viewportSize.height() < rect.bottom()) {
        scrollVertical(rect.bottom() - viewportSize.height());
    }

    // Horizontal scroll
    if (d->horizontal->value() > rect.left()) {
        scrollHorizontal(rect.left());
    } else if (d->horizontal->value() + viewportSize.width() < rect.right()) {
        scrollVertical(rect.right() - viewportSize.width());
    }
} //<

} // namespace Lancelot

